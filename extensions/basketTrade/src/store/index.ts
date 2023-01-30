import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { defineStore } from 'pinia';

interface BasketStoreState {
  currentGlobalBasket: KungfuApi.BasketResolved | null;
  currentGlobalBasketOrder: KungfuApi.BasketOrderResolved | null;
  basketsMap: Record<string, KungfuApi.BasketResolved>;
  basketOrdersMapByLocationId: Record<string, KungfuApi.BasketOrderResolved>;
}

export const useBasketTradeStore = defineStore('basketTrade', {
  state: (): BasketStoreState => ({
    currentGlobalBasket: null,
    currentGlobalBasketOrder: null,
    basketsMap: {},
    basketOrdersMapByLocationId: {},
  }),
  actions: {
    setCurrentGlobalBasket(basket: KungfuApi.BasketResolved | null) {
      this.currentGlobalBasket = basket;
    },

    setCurrentGlobalBasketOrder(
      basketOrder: KungfuApi.BasketOrderResolved | null,
    ) {
      this.currentGlobalBasketOrder = basketOrder;
    },

    setBasketsMap(basketsMap: Record<string, KungfuApi.BasketResolved>) {
      this.basketsMap = basketsMap;
    },

    getBasketById(basketId: string) {
      if (this.basketsMap[basketId]) {
        return this.basketsMap[basketId];
      }

      return null;
    },

    updateBasketOrdersMap(
      curBasketOrdersMap: Record<string, KungfuApi.BasketOrderResolved>,
    ) {
      Object.assign(this.basketOrdersMapByLocationId, curBasketOrdersMap);
    },

    getBasketOrderByOrderLocation(
      basketOrderLocation: KungfuApi.KfExtraLocation,
    ): KungfuApi.BasketOrderResolved | null {
      const basketOrderId = getProcessIdByKfLocation(basketOrderLocation);
      return this.basketOrdersMapByLocationId[basketOrderId];
    },
  },
});
