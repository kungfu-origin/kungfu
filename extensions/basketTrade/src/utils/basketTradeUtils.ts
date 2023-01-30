import {
  h,
  ref,
  Ref,
  computed,
  getCurrentInstance,
  onMounted,
  onBeforeUnmount,
} from 'vue';
import { storeToRefs } from 'pinia';
import { useSubscibeInstrumentAtEntry } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';
import { BasketOrderStatus } from '../components/modules/BasketOrder/config';
import { BasketVolumeType } from '../components/modules/BasketSetting/config';
import { BASKET_CATEGORYS } from '../config';
import {
  hashInstrumentUKey,
  makeOrderByBasketTrade,
  makeOrderByBasketInstruments,
  kfCancelAllOrders,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { getAllBasketInstruments } from '@kungfu-trader/kungfu-js-api/actions';
import {
  BasketVolumeTypeEnum,
  DirectionEnum,
  OrderStatusEnum,
  PriceLevelEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  resolveClientId,
  getProcessIdByKfLocation,
  resolveDirectionBySideAndOffset,
  delayMilliSeconds,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  UnfinishedOrderStatus,
  NotTradeAllOrderStatus,
  ExchangeIds,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  useActiveInstruments,
  useQuote,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { confirmModal } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useBasketTradeStore } from '../store';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { promiseMessageWrapper } from './index';
const { t } = VueI18n.global;

export const buildBasketMapKeyAndLocation = (basket: KungfuApi.Basket) => {
  const basketLocation: KungfuApi.KfExtraLocation = {
    category: BASKET_CATEGORYS.SETTING,
    group: `${basket.id}`,
    name: basket.name,
    mode: 'live',
  };

  return {
    location: basketLocation,
    key: `${basket.id}`,
  };
};

export const dealBasketsToMap = (baskets: KungfuApi.Basket[]) => {
  return baskets.reduce((basketsResolved, curBasket) => {
    const { key, location } = buildBasketMapKeyAndLocation(curBasket);

    basketsResolved[key] = {
      ...curBasket,
      basket_location: location,
      location_id: getProcessIdByKfLocation(location),
      volume_type_resolved: BasketVolumeType[curBasket.volume_type].name,
    };
    return basketsResolved;
  }, {} as Record<string, KungfuApi.BasketResolved>);
};

export const dealBasketInstrumentsToMap = (
  basket: KungfuApi.BasketResolved,
  basketInstruments: KungfuApi.BasketInstrument[],
) => {
  const { getInstrumentByIds } = useActiveInstruments();

  return basketInstruments.reduce(
    (basketInstrumentsResolved, curBasketInstrument) => {
      const { instrument_id, exchange_id, basket_uid } = curBasketInstrument;
      const instrumentResolved = getInstrumentByIds(
        instrument_id,
        exchange_id,
        true,
      ) as KungfuApi.InstrumentResolved;
      const resolvedCurBasketInstrument: KungfuApi.BasketInstrumentResolved = {
        ...curBasketInstrument,
        ...instrumentResolved,
        basketInstrumentName: `${instrument_id} ${ExchangeIds[exchange_id].name} ${instrumentResolved.instrumentName}`,
        basketInstrumentId: `${instrument_id}_${exchange_id}_${new Date().getTime()}`,
      };

      if (!basketInstrumentsResolved[basket_uid]) {
        basketInstrumentsResolved[basket_uid] = {};
      }

      basketInstrumentsResolved[basket_uid][
        resolvedCurBasketInstrument.basketInstrumentId
      ] = resolvedCurBasketInstrument;

      return basketInstrumentsResolved;
    },
    {} as Record<string, Record<string, KungfuApi.BasketInstrumentResolved>>,
  );
};

export const buildBasketOrderMapKeyAndLocation = (
  basketOrder: KungfuApi.BasketOrder,
) => {
  const basketOrderLocation: KungfuApi.KfExtraLocation = {
    category: BASKET_CATEGORYS.ORDER,
    group: `${basketOrder.parent_id}`,
    name: `${basketOrder.order_id}`,
    mode: 'live',
  };

  const locationId = getProcessIdByKfLocation(basketOrderLocation);

  return {
    location: basketOrderLocation,
    key: locationId,
  };
};

export const dealBasketOrdersToMap = (
  basketOrders: KungfuApi.BasketOrder[],
): Record<string, KungfuApi.BasketOrderResolved> => {
  const dealBasketOrderTradeProgress = (
    tradedVolume: bigint,
    totalVolume: bigint,
  ) => {
    return Math.floor((Number(tradedVolume) / Number(totalVolume)) * 100);
  };

  return basketOrders.reduce((basketOrdersMap, curBasketOrder) => {
    const { key, location } = buildBasketOrderMapKeyAndLocation(curBasketOrder);
    const dest_resolved_data = resolveClientId(
      window.watcher,
      curBasketOrder.dest_id,
    );

    basketOrdersMap[key] = {
      ...curBasketOrder,
      dest_resolved_data,
      dest_uname: dest_resolved_data.name,
      basket_order_location: location,
      primary_time_resolved: dealKfTime(curBasketOrder.insert_time),
      status_uname: BasketOrderStatus[curBasketOrder.status].name,
      status_color: BasketOrderStatus[curBasketOrder.status].color || 'default',
      progress: dealBasketOrderTradeProgress(
        curBasketOrder.volume - curBasketOrder.volume_left,
        curBasketOrder.volume,
      ),
    };
    return basketOrdersMap;
  }, {} as Record<string, KungfuApi.BasketOrderResolved>);
};

export const useCurrentGlobalBasket = () => {
  const { currentGlobalBasket, currentGlobalBasketOrder } = storeToRefs(
    useBasketTradeStore(),
  );

  const currentBasketData = computed(() => {
    if (!currentGlobalBasket.value) {
      return null;
    }

    const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
      globalThis.HookKeeper.getHooks().dealTradingData.getCategoryMap();
    return extraCategory[BASKET_CATEGORYS.SETTING];
  });

  const currentBasketOrderData = computed(() => {
    if (!currentGlobalBasketOrder.value) {
      return null;
    }

    const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
      globalThis.HookKeeper.getHooks().dealTradingData.getCategoryMap();
    return extraCategory[BASKET_CATEGORYS.ORDER];
  });

  const dealBasketRowClassName = (basket: KungfuApi.BasketResolved): string => {
    if (!currentGlobalBasket.value) return '';

    if (basket.id === currentGlobalBasket.value.id) {
      return 'current-global-kfLocation';
    }

    return '';
  };

  const dealBasketOrderRowClassName = (
    basketOrder: KungfuApi.BasketOrderResolved,
  ): string => {
    if (!currentGlobalBasketOrder.value) return '';

    if (basketOrder.order_id === currentGlobalBasketOrder.value.order_id) {
      return 'current-global-kfLocation';
    }

    return '';
  };

  const setCurrentGlobalBasket = (basket: KungfuApi.BasketResolved | null) => {
    useBasketTradeStore().setCurrentGlobalBasket(basket);
  };

  const setCurrentGlobalBasketOrder = (
    basketOrder: KungfuApi.BasketOrderResolved | null,
  ) => {
    useBasketTradeStore().setCurrentGlobalBasketOrder(basketOrder);
  };

  return {
    currentGlobalBasket,
    currentGlobalBasketOrder,
    currentBasketData,
    currentBasketOrderData,
    dealBasketRowClassName,
    dealBasketOrderRowClassName,
    setCurrentGlobalBasket,
    setCurrentGlobalBasketOrder,
  };
};

export const useSubscribeBasketInstruments = () => {
  const basketInstrumentForSubGetter = (
    watcher: KungfuApi.Watcher,
  ): KungfuApi.InstrumentForSub[] => {
    return watcher.ledger.BasketInstrument.list().map((basketInstrument) => {
      const uidKey = hashInstrumentUKey(
        basketInstrument.instrument_id,
        basketInstrument.exchange_id,
      );
      return {
        uidKey,
        instrumentId: basketInstrument.instrument_id,
        exchangeId: basketInstrument.exchange_id,
        instrumentName: '',
        instrumentType: basketInstrument.instrument_type,
        id: uidKey,
        ukey: uidKey,
      };
    });
  };

  useSubscibeInstrumentAtEntry(window.watcher, basketInstrumentForSubGetter);
};

export const getBasketInstrumentOrderVolume = (
  basket: KungfuApi.Basket,
  basketInstrument: KungfuApi.BasketInstrument,
  lastPrice: number,
) => {
  if (!lastPrice) return 0;

  if (basket.volume_type === BasketVolumeTypeEnum.Proportion) {
    const basketInstrumentAsset =
      Number(basket.total_amount) * (basketInstrument.rate / 100);
    return Math.floor(basketInstrumentAsset / lastPrice);
  } else if (basket.volume_type === BasketVolumeTypeEnum.Quantity) {
    return Number(basketInstrument.volume);
  } else {
    return 0;
  }
};

export const useBasketQuote = () => {
  const app = getCurrentInstance();
  const { basketsMap, basketOrdersMapByLocationId } = storeToRefs(
    useBasketTradeStore(),
  );

  const basketOrderMarkedValueMap = ref<Record<string, number>>({});

  const basketTotalAmountMap = ref<Record<string, number>>({});

  const getBasketMarkedValue = (basket: KungfuApi.BasketResolved) => {
    return Object.values(basketOrdersMapByLocationId.value)
      .filter((basketOrder) => BigInt(basket.id) === basketOrder.parent_id)
      .reduce((markedValue, basketOrder) => {
        const { key } = buildBasketOrderMapKeyAndLocation(basketOrder);
        markedValue += basketOrderMarkedValueMap.value[key] || 0;
        return markedValue;
      }, 0);
  };

  const getBasketTotalAmount = (basket: KungfuApi.BasketResolved) => {
    return basketTotalAmountMap.value[basket.id] ?? 0;
  };

  onMounted(() => {
    const subscibtion = app?.proxy?.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        basketTotalAmountMap.value = {};
        getAllBasketInstruments().then((basketInstruments) => {
          basketInstruments.forEach((basketInstrument) => {
            const ukey = hashInstrumentUKey(
              basketInstrument.instrument_id,
              basketInstrument.exchange_id,
            );
            const quote = watcher.ledger.Quote[ukey] as KungfuApi.Quote | null;
            const basket = basketsMap.value[
              basketInstrument.basket_uid
            ] as KungfuApi.BasketResolved | null;

            if (quote && basket) {
              basketTotalAmountMap.value[basketInstrument.basket_uid] =
                (basketTotalAmountMap.value[basketInstrument.basket_uid] ?? 0) +
                getBasketInstrumentOrderVolume(
                  basket,
                  basketInstrument,
                  quote.last_price,
                );
            }
          });
        });

        Object.keys(basketOrdersMapByLocationId.value).forEach((key) => {
          const curBasketOrder = basketOrdersMapByLocationId.value[key];

          const orders = watcher.ledger.Order.filter(
            'parent_id',
            curBasketOrder.order_id,
          )
            .nofilter('parent_id', 0n)
            .list();

          basketOrderMarkedValueMap.value[key] = orders.reduce(
            (markedValue, order) => {
              const ukey = hashInstrumentUKey(
                order.instrument_id,
                order.exchange_id,
              );
              const quote = watcher.ledger.Quote[
                ukey
              ] as KungfuApi.Quote | null;

              if (!quote) return markedValue;

              const { last_price } = quote;
              markedValue +=
                Number(order.volume - order.volume_left) * last_price;

              return markedValue;
            },
            0,
          );
        });
      },
    );

    onBeforeUnmount(() => {
      subscibtion?.unsubscribe();
    });
  });

  return {
    getBasketMarkedValue,
    getBasketTotalAmount,
  };
};

export const useMakeBasketOrderFormModal = () => {
  const openModalTag = 'open:MakeBasketOrderModal';
  const confirmModalTag = 'confirm:MakeBasketOrderModal';
  const formStateChangeTag = 'input:currentConfigModal';

  const app = getCurrentInstance();
  const showMakeOrderModal = ref(false);
  const makeBasketOrderConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
    type: 'custom',
    title: t('BasketTrade.place_order'),
    config: {} as KungfuApi.KfExtConfig,
  });
  let handleConfirmModalCallback: ((formState) => void) | undefined;
  let handleFormStateChangeCallback:
    | ((formState, payload: Ref<KungfuApi.SetKfConfigPayload>) => void)
    | undefined;
  const isUseHandleShowModal = ref(false);

  const handleShowMakeBasketOrderModal = (
    title: string,
    settings: KungfuApi.KfConfigItem[],
    handleConfirmModal: (formState) => void | undefined,
    handleFormStateChange?: (
      formState,
      payload: Ref<KungfuApi.SetKfConfigPayload>,
    ) => void,
  ) => {
    isUseHandleShowModal.value = true;
    makeBasketOrderConfigPayload.value.type = 'custom';
    makeBasketOrderConfigPayload.value.title = title;

    makeBasketOrderConfigPayload.value.config = {
      type: [],
      name: title,
      category: BASKET_CATEGORYS.ORDER,
      key: BASKET_CATEGORYS.ORDER,
      extPath: '',
      settings,
    };

    makeBasketOrderConfigPayload.value.initValue = undefined;

    handleConfirmModalCallback = handleConfirmModal;
    handleFormStateChangeCallback = handleFormStateChange;

    triggerShowMakeOrderModal(makeBasketOrderConfigPayload.value);
  };

  const triggerShowMakeOrderModal = (payload: KungfuApi.SetKfConfigPayload) => {
    if (app?.proxy) {
      app.proxy.$globalBus.next({
        tag: openModalTag,
        payload,
      });
    }
  };

  const handleCloseModal = () => {
    isUseHandleShowModal.value = false;
    showMakeOrderModal.value = false;
    makeBasketOrderConfigPayload.value = {
      type: 'custom',
      title: t('BasketTrade.place_order'),
      config: {} as KungfuApi.KfExtConfig,
    };
    handleConfirmModalCallback = undefined;
    handleFormStateChangeCallback = undefined;
  };

  const handleConfirmMakeBasketOrder = (formState) => {
    if (app?.proxy) {
      app.proxy.$globalBus.next({
        tag: confirmModalTag,
        formState,
      });
    }
  };

  onMounted(() => {
    const subscribtion = app?.proxy?.$globalBus.subscribe((data) => {
      if (data.tag === openModalTag && !isUseHandleShowModal.value) {
        makeBasketOrderConfigPayload.value = data.payload;
        showMakeOrderModal.value = true;
      }

      if (data.tag === confirmModalTag && isUseHandleShowModal.value) {
        handleConfirmModalCallback?.(data.formState);
        isUseHandleShowModal.value = false;
      }

      if (
        data.tag === formStateChangeTag &&
        data.category === BASKET_CATEGORYS.ORDER
      ) {
        handleFormStateChangeCallback?.(
          data.formState,
          makeBasketOrderConfigPayload,
        );
      }
    });

    onBeforeUnmount(() => {
      subscribtion?.unsubscribe();
    });
  });

  return {
    showMakeOrderModal,
    handleCloseModal,
    makeBasketOrderConfigPayload,
    handleShowMakeBasketOrderModal,
    handleConfirmMakeBasketOrder,
  };
};

export const useMakeOrCancelBasketOrder = () => {
  const { getQuoteByInstrument } = useQuote();

  const getPriceByPriceOffset = (
    targetPrice: number,
    side: SideEnum,
    priceOffset: number,
  ) => {
    let resolvedPrice = 0;
    if (side === SideEnum.Buy) {
      resolvedPrice = targetPrice + priceOffset;
    } else if (side === SideEnum.Sell) {
      resolvedPrice = targetPrice - priceOffset;
    }

    return resolvedPrice > 0 ? resolvedPrice : 0;
  };

  const getBasketInstrumentOrderPrice = (
    priceLevel: PriceLevelEnum,
    side: SideEnum,
    priceOffset: number,
    instrumentQuote: KungfuApi.Quote,
  ) => {
    switch (priceLevel) {
      case PriceLevelEnum.LowerLimitPrice:
        return getPriceByPriceOffset(
          instrumentQuote.lower_limit_price,
          side,
          priceOffset,
        );
      case PriceLevelEnum.UpperLimitPrice:
        return getPriceByPriceOffset(
          instrumentQuote.upper_limit_price,
          side,
          priceOffset,
        );
      case PriceLevelEnum.Latest:
        return getPriceByPriceOffset(
          instrumentQuote.last_price,
          side,
          priceOffset,
        );
      case PriceLevelEnum.Buy1:
      case PriceLevelEnum.Buy2:
      case PriceLevelEnum.Buy3:
      case PriceLevelEnum.Buy4:
      case PriceLevelEnum.Buy5:
        return getPriceByPriceOffset(
          instrumentQuote.bid_price[priceLevel - 6],
          side,
          priceOffset,
        );
      case PriceLevelEnum.Sell1:
      case PriceLevelEnum.Sell2:
      case PriceLevelEnum.Sell3:
      case PriceLevelEnum.Sell4:
      case PriceLevelEnum.Sell5:
        return getPriceByPriceOffset(
          instrumentQuote.ask_price[priceLevel - 1],
          side,
          priceOffset,
        );
      case PriceLevelEnum.Unknown:
      default:
        return 0;
    }
  };

  const dealBasketInstrumentsToForOrder = (
    basket: KungfuApi.BasketResolved,
    basketInstruments: KungfuApi.BasketInstrumentResolved[],
    priceLevel: PriceLevelEnum,
    side: SideEnum,
    priceOffset: number,
    basketVolume: number,
    ordersMap?: Record<string, KungfuApi.Order>,
  ): KungfuApi.BasketInstrumentForOrder[] => {
    const basketInstrumentsResolved: KungfuApi.BasketInstrumentForOrder[] =
      basketInstruments.map((basketInstrument) => {
        const instrumentQuote = getQuoteByInstrument(basketInstrument);
        const orderPrice = ordersMap
          ? ordersMap[
              buildOrdersMapKey(
                basketInstrument.instrumentId,
                basketInstrument.exchangeId,
                basketInstrument.direction,
              )
            ].limit_price
          : 0;

        if (instrumentQuote) {
          const priceForOrder = getBasketInstrumentOrderPrice(
            priceLevel,
            side,
            priceOffset,
            instrumentQuote,
          );
          return {
            ...basketInstrument,
            volumeResolved:
              getBasketInstrumentOrderVolume(
                basket,
                basketInstrument,
                priceForOrder,
              ) * basketVolume,
            priceResolved: priceForOrder,
            isNoQuote: false,
          };
        } else {
          return {
            ...basketInstrument,
            volumeResolved: 0,
            priceResolved: orderPrice,
            isNoQuote: true,
          };
        }
      });

    return basketInstrumentsResolved;
  };

  const checkBasketInstrumentsQuoteForOrder = (
    basketInstrumentsForOrder: KungfuApi.BasketInstrumentForOrder[],
  ) => {
    const normalBasketInstruments: KungfuApi.BasketInstrumentForOrder[] = [];
    const abnormalBasketInstruments: KungfuApi.BasketInstrumentForOrder[] = [];
    basketInstrumentsForOrder.forEach((basketInstrumentForOrder) => {
      if (basketInstrumentForOrder.isNoQuote) {
        abnormalBasketInstruments.push(basketInstrumentForOrder);
      } else {
        normalBasketInstruments.push(basketInstrumentForOrder);
      }
    });

    return {
      normal: normalBasketInstruments,
      abnormal: abnormalBasketInstruments,
    };
  };

  const getConfirmMakeBasketOrderByCheckQuote = (
    type: 'make' | 'chase' | 'replenish',
    normalBasketInstruments: KungfuApi.BasketInstrumentForOrder[],
    abnormalBasketInstruments: KungfuApi.BasketInstrumentForOrder[],
  ) => {
    if (!abnormalBasketInstruments.length) return Promise.resolve(true);

    const abnormalTextVNode = h(
      'div',
      {},
      t('BasketTrade.abnormal_quote_tip1'),
    );
    const abnormalVNode = h(
      'div',
      { class: 'color-red' },
      abnormalBasketInstruments.map((item) =>
        h(
          'div',
          {},
          `${item.instrumentId} ${item.exchangeId} ${item.instrumentName}`,
        ),
      ),
    );
    const continueTextVNode = h(
      'div',
      {},
      t('BasketTrade.abnormal_quote_tip3'),
    );
    const normalTextVNode = h(
      'div',
      {},
      normalBasketInstruments?.length
        ? t('BasketTrade.abnormal_quote_tip2')
        : t('BasketTrade.abnormal_quote_tip4'),
    );
    const normalVNode = h(
      'div',
      { class: 'color-green' },
      normalBasketInstruments.map((item) =>
        h(
          'div',
          {},
          `${item.instrumentId} ${item.exchangeId} ${item.instrumentName}`,
        ),
      ),
    );
    const context = h('div', {}, [
      abnormalTextVNode,
      abnormalVNode,
      ...(type === 'make'
        ? [normalTextVNode, normalVNode]
        : [continueTextVNode]),
    ]);

    return confirmModal(
      t('BasketTrade.abnormal_quote'),
      context,
      normalBasketInstruments?.length
        ? t('BasketTrade.continue_make_order')
        : undefined,
      t('BasketTrade.cancel_make_order'),
    );
  };

  const makeBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.BasketResolved,
    basketOrderInput: KungfuApi.BasketOrderInput,
    basketInstruments: KungfuApi.BasketInstrumentResolved[],
    accountLocations: KungfuApi.KfLocation[],
    basketVolume: number,
  ) => {
    const basketInstrumentsForOrder = dealBasketInstrumentsToForOrder(
      basket,
      basketInstruments,
      basketOrderInput.price_level,
      basketOrderInput.side,
      basketOrderInput.price_offset,
      basketVolume,
    );

    if (!basketInstrumentsForOrder.length)
      return promiseMessageWrapper(
        Promise.reject(new Error(t('BasketTrade.no_selected_instruments'))),
        { errorTextByError: true },
      );

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote('make', normal, abnormal).then(
      (flag) => {
        if (flag) {
          if (!normal.length) return Promise.resolve();

          const makeBasketOrderPromise = Promise.all(
            accountLocations.map((accountLocation) => {
              return makeOrderByBasketTrade(
                watcher,
                basket,
                basketOrderInput,
                normal,
                accountLocation,
              )
                .then((orderIds) => {
                  if (
                    orderIds &&
                    orderIds.length === basketInstruments.length
                  ) {
                    return Promise.resolve();
                  }

                  return Promise.reject();
                })
                .then(() => delayMilliSeconds(160));
            }),
          );

          return promiseMessageWrapper(makeBasketOrderPromise, {
            errorTextByError: true,
          });
        }

        return Promise.resolve();
      },
    );
  };

  const buildOrdersMapKey = (
    instrumentId: string,
    exchangeId: string,
    direction: DirectionEnum,
  ) => {
    return `${instrumentId}_${exchangeId}_${direction}`;
  };

  const defaultOrdersMapKeyBuilder = (order: KungfuApi.Order) =>
    buildOrdersMapKey(
      order.instrument_id,
      order.exchange_id,
      resolveDirectionBySideAndOffset(order.side, order.offset),
    );

  const getBasketOrderTargetStatusOrdersMap = <
    T extends Record<string, OrderStatusEnum[]>,
  >(
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
    mapKeyBuilter: (order: KungfuApi.Order) => string,
    targetStatus: T,
  ): {
    [K in keyof T]: Record<string, KungfuApi.Order>;
  } => {
    return (
      globalThis.HookKeeper.getHooks().dealTradingData.trigger(
        watcher,
        basketOrder.basket_order_location,
        watcher.ledger.Order,
        'order',
      ) as KungfuApi.Order[]
    ).reduce(
      (map, item) => {
        const key = mapKeyBuilter(item);
        Object.keys(targetStatus).forEach((statusKey) => {
          const targetStatusKey = statusKey as keyof T;
          if (!map[targetStatusKey]) map[targetStatusKey] = {};
          if (targetStatus[targetStatusKey].includes(item.status)) {
            const targetMap = map[targetStatusKey] as Record<
              string,
              KungfuApi.Order
            >;
            if (key in targetMap) return;
            targetMap[key] = item;
          }
        });
        return map;
      },
      {} as {
        [K in keyof T]: Record<string, KungfuApi.Order>;
      },
    );
  };

  const getBasketOrderUnfinishedOrdersMap = (
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
  ) => {
    const { unfinished, filled, notTradeAll } =
      getBasketOrderTargetStatusOrdersMap(
        watcher,
        basketOrder,
        defaultOrdersMapKeyBuilder,
        {
          unfinished: UnfinishedOrderStatus,
          filled: [OrderStatusEnum.Filled],
          notTradeAll: NotTradeAllOrderStatus,
        },
      );
    const unfinishedOrderKeys = Object.keys(unfinished);
    if (unfinishedOrderKeys.length) {
      return unfinishedOrderKeys.reduce((ordersMap, key) => {
        if (key in filled) {
          const volumeLeft = unfinishedOrderKeys[key].volume_left;
          const tradedVolume = filled[key].volume - filled[key].volume_left;
          if (volumeLeft === tradedVolume) {
            return ordersMap;
          }

          if (
            key in notTradeAll &&
            notTradeAll[key].volume_left <= filled[key].volume_left
          ) {
            if (notTradeAll[key].volume_left === tradedVolume) {
              return ordersMap;
            }
          }
        }

        ordersMap[key] = unfinished[key];

        return ordersMap;
      }, {} as Record<string, KungfuApi.Order>);
    }

    return unfinished || {};
  };

  const cancalBasketOrder = (
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
  ) => {
    const resolvedOrdersMap = getBasketOrderUnfinishedOrdersMap(
      watcher,
      basketOrder,
    );
    const ordersResolved = Object.values(resolvedOrdersMap ?? {});

    if (!ordersResolved.length)
      return promiseMessageWrapper(
        Promise.reject(
          new Error(t('BasketTrade.basket_order_no_failed_orders')),
        ),
        { errorTextByError: true },
      );

    const cancalOrdersPromise = kfCancelAllOrders(watcher, ordersResolved).then(
      (canceledOrders) => {
        if (canceledOrders && canceledOrders.length === ordersResolved.length) {
          return Promise.resolve();
        }

        return Promise.reject();
      },
    );

    return promiseMessageWrapper(cancalOrdersPromise, {
      errorTextByError: true,
    });
  };

  const getBasketInstrumentsForOrder = (
    orders: KungfuApi.Order[],
    basket: KungfuApi.BasketResolved,
    basketOrder: KungfuApi.BasketOrderResolved,
  ): KungfuApi.BasketInstrumentForOrder[] => {
    return orders.map((order) => {
      const ukey = hashInstrumentUKey(order.instrument_id, order.exchange_id);
      const curBasketInstrument: KungfuApi.BasketInstrumentResolved = {
        basket_uid: basket.id,
        instrument_id: order.instrument_id,
        exchange_id: order.exchange_id,
        instrument_type: order.instrument_type,
        instrumentId: order.instrument_id,
        exchangeId: order.exchange_id,
        instrumentType: order.instrument_type,
        instrumentName: '',
        id: ukey,
        ukey,
        direction: resolveDirectionBySideAndOffset(order.side, order.offset),
        volume: order.volume_left,
        rate: 0,
        basketInstrumentId: '',
        basketInstrumentName: '',
      };

      const instrumentQuote = getQuoteByInstrument(curBasketInstrument);

      return {
        ...curBasketInstrument,
        volumeResolved: Number(order.volume_left),
        priceResolved: instrumentQuote
          ? getBasketInstrumentOrderPrice(
              basketOrder.price_level,
              basketOrder.side,
              basketOrder.price_offset,
              instrumentQuote,
            )
          : order.limit_price,
        isNoQuote: !instrumentQuote,
      };
    });
  };

  const chaseBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.BasketResolved,
    basketOrder: KungfuApi.BasketOrderResolved,
    orders?: KungfuApi.OrderResolved[],
  ) => {
    let ordersResolved: KungfuApi.Order[] = [];

    if (orders) {
      ordersResolved = orders.filter((order) =>
        UnfinishedOrderStatus.includes(order.status),
      );
    } else {
      const resolvedOrdersMap = getBasketOrderUnfinishedOrdersMap(
        watcher,
        basketOrder,
      );

      ordersResolved = Object.values(resolvedOrdersMap);
    }

    if (!ordersResolved.length)
      return promiseMessageWrapper(
        Promise.reject(
          new Error(t('BasketTrade.basket_order_no_failed_orders')),
        ),
        { errorTextByError: true },
      );

    const { notTradeAll } = getBasketOrderTargetStatusOrdersMap(
      watcher,
      basketOrder,
      defaultOrdersMapKeyBuilder,
      { notTradeAll: UnfinishedOrderStatus },
    );

    ordersResolved = ordersResolved.map((order) => {
      const orderKey = buildOrdersMapKey(
        order.instrument_id,
        order.exchange_id,
        resolveDirectionBySideAndOffset(order.side, order.offset),
      );
      if (
        orderKey in notTradeAll &&
        notTradeAll[orderKey].volume_left <= order.volume_left
      ) {
        order.volume_left = notTradeAll[orderKey].volume_left;
      }
      return order;
    });

    const basketInstrumentsForOrder = getBasketInstrumentsForOrder(
      ordersResolved,
      basket,
      basketOrder,
    );

    if (!basketInstrumentsForOrder.length)
      return Promise.reject(new Error(t('BasketTrade.no_pending_orders')));

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote(
      'chase',
      normal,
      abnormal,
    ).then((flag) => {
      if (flag) {
        const chaseBasketOrderPromise = kfCancelAllOrders(
          watcher,
          ordersResolved,
        ).then((canceledOrderIds) => {
          if (
            canceledOrderIds &&
            canceledOrderIds.length !== ordersResolved.length
          ) {
            return Promise.reject();
          }

          const tdLocation = watcher.getLocation(ordersResolved[0].source);

          return makeOrderByBasketInstruments(
            watcher,
            basketOrder.order_id,
            basketOrder,
            basketInstrumentsForOrder,
            tdLocation,
          );
        });

        return promiseMessageWrapper(chaseBasketOrderPromise, {
          errorTextByError: true,
        });
      }

      return Promise.resolve();
    });
  };

  const getBasketOrderNotTradeAllOrdersMap = (
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
  ) => {
    const { notTradeAll, filled, unfinished } =
      getBasketOrderTargetStatusOrdersMap(
        watcher,
        basketOrder,
        defaultOrdersMapKeyBuilder,
        {
          notTradeAll: NotTradeAllOrderStatus,
          filled: [OrderStatusEnum.Filled],
          unfinished: UnfinishedOrderStatus,
        },
      );
    const notTradeAllKeys = Object.keys(notTradeAll);
    if (notTradeAllKeys.length) {
      return notTradeAllKeys.reduce((ordersMap, key) => {
        if (key in filled) {
          const volumeLeft = notTradeAll[key].volume_left;
          const tradedVolume = filled[key].volume - filled[key].volume_left;
          if (volumeLeft === tradedVolume) {
            return ordersMap;
          }

          if (key in unfinished) {
            const unfinshedVolumeLeft = unfinished[key].volume_left;
            if (unfinshedVolumeLeft === tradedVolume) {
              return ordersMap;
            }
          }
        }

        ordersMap[key] = notTradeAll[key];

        return ordersMap;
      }, {} as Record<string, KungfuApi.Order>);
    }

    return notTradeAll || {};
  };

  const replenishBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.BasketResolved,
    basketOrder: KungfuApi.BasketOrderResolved,
    orders?: KungfuApi.OrderResolved[],
  ) => {
    let ordersResolved: KungfuApi.Order[] = [];

    if (orders) {
      ordersResolved = orders.filter((order) =>
        NotTradeAllOrderStatus.includes(order.status),
      );
    } else {
      const resolvedOrdersMap = getBasketOrderNotTradeAllOrdersMap(
        watcher,
        basketOrder,
      );

      ordersResolved = Object.values(resolvedOrdersMap);
    }

    if (!ordersResolved.length)
      return promiseMessageWrapper(
        Promise.reject(
          new Error(
            orders
              ? t('BasketTrade.no_selected_failed_orders')
              : t('BasketTrade.basket_order_no_failed_orders'),
          ),
        ),
        { errorTextByError: true },
      );

    const { unfinished } = getBasketOrderTargetStatusOrdersMap(
      watcher,
      basketOrder,
      defaultOrdersMapKeyBuilder,
      { unfinished: UnfinishedOrderStatus },
    );

    ordersResolved = ordersResolved.map((order) => {
      const orderKey = buildOrdersMapKey(
        order.instrument_id,
        order.exchange_id,
        resolveDirectionBySideAndOffset(order.side, order.offset),
      );
      if (
        orderKey in unfinished &&
        unfinished[orderKey].volume_left <= order.volume_left
      ) {
        order.volume_left = unfinished[orderKey].volume_left;
      }
      return order;
    });

    const basketInstrumentsForOrder = getBasketInstrumentsForOrder(
      ordersResolved,
      basket,
      basketOrder,
    );

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote(
      'replenish',
      normal,
      abnormal,
    ).then((flag) => {
      if (flag) {
        const tdLocation = watcher.getLocation(ordersResolved[0].source);

        const replenishBasketOrderPromise = makeOrderByBasketInstruments(
          watcher,
          basketOrder.order_id,
          basketOrder,
          basketInstrumentsForOrder,
          tdLocation,
        );

        return promiseMessageWrapper(replenishBasketOrderPromise, {
          errorTextByError: true,
        });
      }

      return Promise.resolve();
    });
  };

  return {
    makeBasketOrder,
    cancalBasketOrder,
    chaseBasketOrder,
    replenishBasketOrder,
    getBasketOrderUnfinishedOrdersMap,
    getBasketOrderNotTradeAllOrdersMap,
  };
};
