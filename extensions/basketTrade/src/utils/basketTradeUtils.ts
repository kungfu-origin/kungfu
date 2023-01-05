import { BasketOrderStatus } from '../components/modules/BasketOrder/config';
import { getIdByKfLocation } from '../../../../framework/api/src/utils/busiUtils';
import { computed, ref, watch, getCurrentInstance, onMounted } from 'vue';
import { useCurrentGlobalKfLocation } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { BasketVolumeType } from '../components/modules/BasketSetting/config';
import { BASKET_CATEGORYS } from '../config';
import {
  basketStore,
  basketInstrumentStore,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  dealCategory,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useActiveInstruments } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { useBasketTradeStore } from '../store';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
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

export const dealBasketsToMap = (baskets: KungfuApi.Basket[]) => {
  return baskets.reduce((basketsResolved, curBasket) => {
    const basketLocation: KungfuApi.KfExtraLocation = {
      category: BASKET_CATEGORYS.SETTING,
      group: `${curBasket.id}`,
      name: curBasket.name,
      mode: 'live',
    };
    const locationId = getProcessIdByKfLocation(basketLocation);
    basketsResolved[locationId] = {
      ...curBasket,
      basket_location: basketLocation,
      location_id: locationId,
      volume_type_resolved: BasketVolumeType[curBasket.volume_type].name,
      marked_value: 0,
    };
    return basketsResolved;
  }, {} as Record<string, KungfuApi.BasketResolved>);
};

export const dealBasketInstrumentsToMap = (
  baskets: KungfuApi.BasketInstrument[],
) => {
  const { getInstrumentByIds } = useActiveInstruments();

  return baskets.reduce((basketInstrumentsResolved, curBasketInstrument) => {
    const { instrument_id, exchange_id, basket_uid } = curBasketInstrument;
    const instrumentResolved = getInstrumentByIds(
      instrument_id,
      exchange_id,
      true,
    ) as KungfuApi.InstrumentResolved;
    const resolvedCurBasketInstrument = {
      ...curBasketInstrument,
      ...instrumentResolved,
      id: `${instrument_id}_${exchange_id}_${new Date().getTime()}`,
    };

    if (!basketInstrumentsResolved[basket_uid]) {
      basketInstrumentsResolved[basket_uid] = {};
    }

    basketInstrumentsResolved[basket_uid][resolvedCurBasketInstrument.id] =
      resolvedCurBasketInstrument;

    return basketInstrumentsResolved;
  }, {} as Record<string, Record<string, KungfuApi.BasketInstrumentResolved>>);
};

export const useCurrentGlobalBasket = () => {
  const { currentGlobalKfLocation, setCurrentGlobalKfLocation } =
    useCurrentGlobalKfLocation(window.watcher);
  const { getBasketByLocation, getBasketOrderByOrderLocation } =
    useBasketTradeStore();

  const currentGlobalBasket = ref<KungfuApi.BasketResolved>();
  const currentGlobalBasketOrder = ref<KungfuApi.BasketOrderResolved>();

  watch(
    () => currentGlobalKfLocation.value,
    (newLocation) => {
      if (!newLocation) return;

      if (newLocation.category === BASKET_CATEGORYS.SETTING) {
        const basket = getBasketByLocation(newLocation);
        basket && (currentGlobalBasket.value = basket);
      }

      if (newLocation.category === BASKET_CATEGORYS.ORDER) {
        const basketOrder = getBasketOrderByOrderLocation(newLocation);
        basketOrder && (currentGlobalBasketOrder.value = basketOrder);
      }
    },
  );

  const currentBasketData = computed(() => {
    if (!currentGlobalKfLocation.value) {
      return null;
    }

    const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
      globalThis.HookKeeper.getHooks().dealTradingData.getCategoryMap();
    return dealCategory(currentGlobalKfLocation.value?.category, extraCategory);
  });

  const setCurrentGlobalBasket = (basket: KungfuApi.BasketResolved) => {
    setCurrentGlobalKfLocation(basket.basket_location);
  };

  const isCurrentGlobalLocationTargetCategory = (
    targetCategory: keyof typeof BASKET_CATEGORYS,
  ) => {
    if (!currentGlobalKfLocation.value) return false;

    return currentGlobalKfLocation.value.category === targetCategory;
  };

  return {
    currentGlobalBasket,
    currentBasketData,
    setCurrentGlobalBasket,
    isCurrentGlobalLocationTargetCategory,
  };
};

export const dealBasketOrdersToMap = (
  basketOrders: KungfuApi.BasketOrder[],
): Record<string, KungfuApi.BasketOrderResolved> => {
  return basketOrders.reduce((basketOrdersMap, curBasketOrder) => {
    const basketOrderLocation: KungfuApi.KfExtraLocation = {
      category: BASKET_CATEGORYS.ORDER,
      group: `${curBasketOrder.parent_id}`,
      name: `${curBasketOrder.order_id}`,
      mode: 'live',
    };

    const locationId = getIdByKfLocation(basketOrderLocation);

    const dealBasketOrderTradeProgress = (
      tradedVolume: bigint,
      totalVolume: bigint,
    ) => {
      return Number((Number(tradedVolume) / Number(totalVolume)).toFixed(2));
    };

    basketOrdersMap[locationId] = {
      ...curBasketOrder,
      basket_order_location: basketOrderLocation,
      price_resolved: 0,
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

export const useMakeBasketOrder = (args?: {
  handleConfirmModal?: (formState) => void;
}) => {
  const openModalTag = 'open:MakeBasketOrderModal';
  const confirmModalTag = 'confirm:MakeBasketOrderModal';

  const app = getCurrentInstance();
  const showMakeOrderModal = ref(false);
  const makeBasketOrderConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
    type: 'custom',
    title: t('BasketTrade.place_order'),
    config: {} as KungfuApi.KfExtConfig,
  });

  const handleShowMakeBasketOrderModal = (
    title: string,
    settings: KungfuApi.KfConfigItem[],
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
    if (app?.proxy) {
      app.proxy.$globalBus.subscribe((data) => {
        if (data.tag === openModalTag) {
          console.error(new Error(data));
          makeBasketOrderConfigPayload.value = data.payload;
          showMakeOrderModal.value = true;
        }

        if (data.tag === confirmModalTag) {
          args?.handleConfirmModal?.(data.formState);
        }
      });
    }
  });

  return {
    showMakeOrderModal,
    makeBasketOrderConfigPayload,
    handleShowMakeBasketOrderModal,
    handleConfirmMakeBasketOrder,
  };
};
