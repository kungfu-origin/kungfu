import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { defineStore } from 'pinia';

interface BasketStoreState {
  basketsMap: Record<string, KungfuApi.BasketResolved>;
  basketOrdersMapByBasket: Record<
    string,
    Record<string, KungfuApi.BasketOrderResolved>
  >;
  basketOrdersMapByOrderId: Record<string, KungfuApi.BasketOrderResolved>;
}

export const useBasketTradeStore = defineStore('basketTrade', {
  state: (): BasketStoreState => ({
    basketsMap: {},
    basketOrdersMapByBasket: {},
    basketOrdersMapByOrderId: {},
  }),
  actions: {
    setBasketsMap(basketsMap: Record<string, KungfuApi.BasketResolved>) {
      this.basketsMap = basketsMap;
    },

    getBasketByLocation(basketLocation: KungfuApi.KfExtraLocation) {
      const basketLocationId = getProcessIdByKfLocation(basketLocation);

      if (this.basketsMap[basketLocationId]) {
        return this.basketsMap[basketLocationId];
      }

      return null;
    },

    updateBasketOrdersMap(
      curBasketOrdersMap: Record<
        string,
        Record<string, KungfuApi.BasketOrderResolved>
      >,
    ) {
      Object.keys(curBasketOrdersMap).forEach((basketId) => {
        if (basketId in this.basketOrdersMapByBasket) {
          Object.assign(
            this.basketOrdersMapByBasket[basketId],
            curBasketOrdersMap[basketId],
          );
        } else {
          this.basketOrdersMapByBasket[basketId] = curBasketOrdersMap[basketId];
        }

        Object.assign(
          this.basketOrdersMapByOrderId,
          curBasketOrdersMap[basketId],
        );
      });
    },

    getBasketOrdersByBasketLocation(basketLocation: KungfuApi.KfExtraLocation) {
      const basket = this.getBasketByLocation(basketLocation);
      if (!basket) return {};

      const curOrdersMap = this.basketOrdersMapByBasket[basket.id];
      if (!curOrdersMap) return {};

      return curOrdersMap;
    },

    getBasketOrderByOrderLocation(
      basketOrderLocation: KungfuApi.KfExtraLocation,
    ): KungfuApi.BasketOrderResolved | null {
      const basketOrderId = getProcessIdByKfLocation(basketOrderLocation);
      return this.basketOrdersMapByOrderId[basketOrderId];
    },
  },
});
