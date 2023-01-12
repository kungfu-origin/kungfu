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
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';
import { BasketOrderPriceTypeEnum } from './../config/makeBasketOrderFormConfig';
import { BasketOrderStatus } from '../components/modules/BasketOrder/config';
import { BasketVolumeType } from '../components/modules/BasketSetting/config';
import { BASKET_CATEGORYS } from '../config';
import {
  basketStore,
  basketInstrumentStore,
  hashInstrumentUKey,
  makeOrderByBasketTrade,
  makeOrderByBasketInstruments,
  kfCancelAllOrders,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  BasketVolumeTypeEnum,
  OrderStatusEnum,
  PriceLevelEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  UnfinishedOrderStatus,
  NotTradeAllOrderStatus,
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

export const getAllBaskets = (): Promise<KungfuApi.Basket[]> => {
  const baskets = basketStore.getAllBasket();
  if (baskets) {
    return Promise.resolve(baskets);
  }
  return Promise.resolve([]);
};

export const setAllBaskets = (baskets: KungfuApi.Basket[]) => {
  return Promise.resolve(basketStore.setAllBasket(baskets));
};

export const getAllBasketInstruments = (): Promise<
  KungfuApi.BasketInstrument[]
> => {
  const basketInstruments = basketInstrumentStore.getAllBasketInstrument();
  if (basketInstruments) {
    return Promise.resolve(basketInstruments);
  }
  return Promise.resolve([]);
};

export const setAllBasketInstruments = (
  basketInstruments: KungfuApi.BasketInstrument[],
) => {
  return Promise.resolve(
    basketInstrumentStore.setAllBasketInstrument(basketInstruments),
  );
};

export const buildBasketMapKeyAndLocation = (basket: KungfuApi.Basket) => {
  const basketLocation: KungfuApi.KfExtraLocation = {
    category: BASKET_CATEGORYS.SETTING,
    group: `${basket.id}`,
    name: basket.name,
    mode: 'live',
  };
  const locationId = getProcessIdByKfLocation(basketLocation);

  return {
    location: basketLocation,
    key: locationId,
  };
};

export const dealBasketsToMap = (baskets: KungfuApi.Basket[]) => {
  return baskets.reduce((basketsResolved, curBasket) => {
    const { key, location } = buildBasketMapKeyAndLocation(curBasket);

    basketsResolved[key] = {
      ...curBasket,
      basket_location: location,
      location_id: key,
      volume_type_resolved: BasketVolumeType[curBasket.volume_type].name,
    };
    return basketsResolved;
  }, {} as Record<string, KungfuApi.BasketResolved>);
};

export const getBasketInstrumentVolume = (
  volumeType: BasketVolumeTypeEnum,
  volume: bigint,
  rate: number,
  totalVolume: bigint,
) => {
  if (volumeType === BasketVolumeTypeEnum.Quantity) {
    return Number(volume);
  } else if (volumeType === BasketVolumeTypeEnum.Proportion) {
    return Math.floor((Number(totalVolume) * rate) / 100);
  }

  return 0;
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
        basketInstrumentName: `${instrument_id} ${instrumentResolved.instrumentName}`,
        basketInstrumentId: `${instrument_id}_${exchange_id}_${new Date().getTime()}`,
        volumeResolved: getBasketInstrumentVolume(
          basket.volume_type,
          curBasketInstrument.volume,
          curBasketInstrument.rate,
          basket.total_volume,
        ),
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
    return Number(
      ((Number(tradedVolume) / Number(totalVolume)) * 100).toFixed(2),
    );
  };

  return basketOrders.reduce((basketOrdersMap, curBasketOrder) => {
    const { key, location } = buildBasketOrderMapKeyAndLocation(curBasketOrder);

    basketOrdersMap[key] = {
      ...curBasketOrder,
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
  const { setCurrentGlobalKfLocation } = useGlobalStore();
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
    setCurrentGlobalKfLocation(basket ? basket.basket_location : null);
    useBasketTradeStore().setCurrentGlobalBasket(basket);
  };

  const setCurrentGlobalBasketOrder = (
    basketOrder: KungfuApi.BasketOrderResolved | null,
  ) => {
    setCurrentGlobalKfLocation(
      basketOrder ? basketOrder.basket_order_location : null,
    );
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

export const useBasketMarkedValue = () => {
  const app = getCurrentInstance();
  const { basketOrdersMapByLocationId } = storeToRefs(useBasketTradeStore());

  const basketOrderMarkedValueMap = ref<Record<string, number>>({});

  const getBasketMarkedValue = (basket: KungfuApi.BasketResolved) => {
    return Object.values(basketOrdersMapByLocationId.value)
      .filter((basketOrder) => BigInt(basket.id) === basketOrder.parent_id)
      .reduce((markedValue, basketOrder) => {
        const { key } = buildBasketOrderMapKeyAndLocation(basketOrder);
        markedValue += basketOrderMarkedValueMap.value[key] || 0;
        return markedValue;
      }, 0);
  };

  onMounted(() => {
    const subscibtion = app?.proxy?.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
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

  const handleShowMakeBasketOrderModal = (
    title: string,
    settings: KungfuApi.KfConfigItem[],
    handleConfirmModal: (formState) => void | undefined,
    handleFormStateChange?: (
      formState,
      payload: Ref<KungfuApi.SetKfConfigPayload>,
    ) => void,
  ) => {
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
      if (data.tag === openModalTag) {
        makeBasketOrderConfigPayload.value = data.payload;
        showMakeOrderModal.value = true;
      }

      if (data.tag === confirmModalTag) {
        handleConfirmModalCallback?.(data.formState);
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
      case PriceLevelEnum.LowLimitPrice:
        return getPriceByPriceOffset(
          instrumentQuote.lower_limit_price,
          side,
          priceOffset,
        );
      case PriceLevelEnum.UpLimitPrice:
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
    basketInstruments: KungfuApi.BasketInstrumentResolved[],
    priceLevel: PriceLevelEnum,
    side: SideEnum,
    priceOffset: number,
    ordersMap?: Record<string, KungfuApi.Order>,
  ): KungfuApi.BasketInstrumentForOrder[] => {
    const basketInstrumentsResolved: KungfuApi.BasketInstrumentForOrder[] =
      basketInstruments.map((basketInstrument) => {
        if (ordersMap) {
          return {
            ...basketInstrument,
            priceResolved:
              ordersMap[
                buildOrdersMapKey(
                  basketInstrument.instrumentId,
                  basketInstrument.exchangeId,
                )
              ].limit_price,
          };
        } else {
          const instrumentQuote = getQuoteByInstrument(basketInstrument);
          return {
            ...basketInstrument,
            priceResolved: instrumentQuote
              ? getBasketInstrumentOrderPrice(
                  priceLevel,
                  side,
                  priceOffset,
                  instrumentQuote,
                )
              : 0,
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
      if (basketInstrumentForOrder.priceResolved === 0) {
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
    const normalTextVNode = h('div', {}, t('BasketTrade.abnormal_quote_tip2'));
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
      normalTextVNode,
      normalVNode,
    ]);

    return confirmModal(
      t('BasksetTrade.abnormal_quote'),
      context,
      t('BasketTrade.continue_make_order'),
      t('BasketTrade.cancel_make_order'),
    );
  };

  const makeBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.Basket,
    basketOrderInput: KungfuApi.BasketOrderInput,
    basketInstruments: KungfuApi.BasketInstrumentResolved[],
    accountLocation: KungfuApi.KfLocation,
  ) => {
    const basketInstrumentsForOrder = dealBasketInstrumentsToForOrder(
      basketInstruments,
      basketOrderInput.price_level,
      basketOrderInput.side,
      basketOrderInput.price_offset,
    );

    if (!basketInstrumentsForOrder.length)
      return promiseMessageWrapper(
        Promise.reject(t('BasketTrade.no_selected_instruments')),
        { errorTextByError: true },
      );

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote(normal, abnormal).then(
      (flag) => {
        if (flag) {
          const makeBasketOrderPromise = makeOrderByBasketTrade(
            watcher,
            basket,
            basketOrderInput,
            basketInstrumentsForOrder,
            accountLocation,
          ).then((orderIds) => {
            if (orderIds.length === basketInstruments.length) {
              return Promise.resolve();
            }

            return Promise.reject();
          });

          return promiseMessageWrapper(makeBasketOrderPromise, {
            errorTextByError: true,
          });
        }

        return Promise.resolve();
      },
    );
  };

  const buildOrdersMapKey = (instrumentId: string, exchangeId: string) => {
    return `${instrumentId}_${exchangeId}`;
  };

  const getBasketOrderTargetStatusOrdersMap = (
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
    targetStatus: OrderStatusEnum[],
  ) => {
    return (
      globalThis.HookKeeper.getHooks().dealTradingData.trigger(
        watcher,
        basketOrder.basket_order_location,
        watcher.ledger.Order,
        'order',
      ) as KungfuApi.Order[]
    ).reduce((map, item) => {
      if (targetStatus.includes(item.status)) {
        map[buildOrdersMapKey(item.instrument_id, item.exchange_id)] = item;
      }
      return map;
    }, {} as Record<string, KungfuApi.Order>);
  };

  const cancalBasketOrder = (
    watcher: KungfuApi.Watcher,
    basketOrder: KungfuApi.BasketOrderResolved,
  ) => {
    const resolvedOrdersMap = getBasketOrderTargetStatusOrdersMap(
      watcher,
      basketOrder,
      UnfinishedOrderStatus,
    );
    const ordersResolved = Object.values(resolvedOrdersMap);

    if (!ordersResolved.length)
      return promiseMessageWrapper(
        Promise.reject(t('BasketTrade.basket_order_no_failed_orders')),
        { errorTextByError: true },
      );

    const cancalOrdersPromise = kfCancelAllOrders(watcher, ordersResolved).then(
      (canceledOrders) => {
        if (canceledOrders.length === ordersResolved.length) {
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
    isOrderPrice: boolean,
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
        volume: order.volume_left,
        rate: 0,
        basketInstrumentId: '',
        basketInstrumentName: '',
        volumeResolved: Number(order.volume_left),
      };

      if (isOrderPrice) {
        return {
          ...curBasketInstrument,
          priceResolved: order.limit_price,
        };
      } else {
        const instrumentQuote = getQuoteByInstrument(curBasketInstrument);

        return {
          ...curBasketInstrument,
          priceResolved: instrumentQuote
            ? getBasketInstrumentOrderPrice(
                basketOrder.price_level,
                basketOrder.side,
                basketOrder.price_offset,
                instrumentQuote,
              )
            : 0,
        };
      }
    });
  };

  const chaseBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.BasketResolved,
    basketOrder: KungfuApi.BasketOrderResolved,
    basketOrderPriceType: BasketOrderPriceTypeEnum,
  ) => {
    const resolvedOrdersMap = getBasketOrderTargetStatusOrdersMap(
      watcher,
      basketOrder,
      UnfinishedOrderStatus,
    );
    const ordersResolved = Object.values(resolvedOrdersMap);

    if (!ordersResolved.length)
      return promiseMessageWrapper(
        Promise.reject(t('BasketTrade.basket_order_no_failed_orders')),
        { errorTextByError: true },
      );

    const basketInstrumentsForOrder = getBasketInstrumentsForOrder(
      ordersResolved,
      basket,
      basketOrder,
      basketOrderPriceType === BasketOrderPriceTypeEnum.ORDER,
    );

    if (!basketInstrumentsForOrder.length)
      return Promise.reject(new Error(t('BasketTrade.no_pending_orders')));

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote(normal, abnormal).then(
      (flag) => {
        if (flag) {
          const chaseBasketOrderPromise = kfCancelAllOrders(
            watcher,
            ordersResolved,
          ).then((canceledOrderIds) => {
            if (canceledOrderIds.length !== ordersResolved.length) {
              return Promise.reject();
            }

            const tdLocation = watcher.getLocation(ordersResolved[0].source);

            return makeOrderByBasketInstruments(
              watcher,
              basketOrder.order_id,
              basketOrder,
              normal,
              tdLocation,
            );
          });

          return promiseMessageWrapper(chaseBasketOrderPromise, {
            errorTextByError: true,
          });
        }

        return Promise.resolve();
      },
    );
  };

  const replenishBasketOrder = (
    watcher: KungfuApi.Watcher,
    basket: KungfuApi.BasketResolved,
    basketOrder: KungfuApi.BasketOrderResolved,
    basketOrderPriceType: BasketOrderPriceTypeEnum,
    orders?: KungfuApi.OrderResolved[],
  ) => {
    let ordersResolved: KungfuApi.Order[] = [];

    if (orders) {
      ordersResolved = orders.filter((order) =>
        NotTradeAllOrderStatus.includes(order.status),
      );
    } else {
      const resolvedOrdersMap = getBasketOrderTargetStatusOrdersMap(
        watcher,
        basketOrder,
        NotTradeAllOrderStatus,
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

    const basketInstrumentsForOrder = getBasketInstrumentsForOrder(
      ordersResolved,
      basket,
      basketOrder,
      basketOrderPriceType === BasketOrderPriceTypeEnum.ORDER,
    );

    const { normal, abnormal } = checkBasketInstrumentsQuoteForOrder(
      basketInstrumentsForOrder,
    );

    return getConfirmMakeBasketOrderByCheckQuote(normal, abnormal).then(
      (flag) => {
        if (flag) {
          const tdLocation = watcher.getLocation(ordersResolved[0].source);

          const replenishBasketOrderPromise = makeOrderByBasketInstruments(
            watcher,
            basketOrder.order_id,
            basketOrder,
            normal,
            tdLocation,
          );

          return promiseMessageWrapper(replenishBasketOrderPromise, {
            errorTextByError: true,
          });
        }

        return Promise.resolve();
      },
    );
  };

  return {
    makeBasketOrder,
    cancalBasketOrder,
    chaseBasketOrder,
    replenishBasketOrder,
  };
};
