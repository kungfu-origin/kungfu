import { getUIComponents } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { App, defineAsyncComponent } from 'vue';

import { Router } from 'vue-router';
import { useGlobalStore } from './store/global';

import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

export const useComponenets = (
  app: App<Element>,
  router: Router,
): Promise<void> => {
  app.component(
    t('Pos.name'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/pos/Pos.vue'),
    ),
  );

  app.component(
    t('PosGlobal.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/posGlobal/PosGlobal.vue'
        ),
    ),
  );

  app.component(
    t('Order.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/order/Order.vue'
        ),
    ),
  );

  app.component(
    t('Trade.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/trade/Trade.vue'
        ),
    ),
  );

  app.component(
    t('Td.name'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/td/Td.vue'),
    ),
  );

  app.component(
    t('Md.name'),
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/md/Md.vue'),
    ),
  );

  app.component(
    t('Strategy.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/strategy/Strategy.vue'
        ),
    ),
  );

  app.component(
    t('TradingTask.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/tradingTask/TradingTask.vue'
        ),
    ),
  );

  app.component(
    t('MarketData.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/marketdata/MarketData.vue'
        ),
    ),
  );

  app.component(
    t('OrderBook.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/orderBook/OrderBook.vue'
        ),
    ),
  );

  app.component(
    t('MakeOrderDashboard.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/makeOrder/MakeOrderDashboard.vue'
        ),
    ),
  );

  app.component(
    t('FutureArbitrage.name'),
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/futureArbitrage/FutureArbitrage.vue'
        ),
    ),
  );

  app.config.globalProperties.$availKfBoards = [
    t('Pos.name'),
    t('PosGlobal.name'),
    t('Order.name'),
    t('Trade.name'),
    t('Td.name'),
    t('Md.name'),
    t('Strategy.name'),
    t('TradingTask.name'),
    t('MarketData.name'),
    t('OrderBook.name'),
    t('MakeOrderDashboard.name'),
    t('FutureArbitrage.name'),
  ];

  return useGlobalStore()
    .setKfUIExtConfigs()
    .then((configs) => getUIComponents(configs))
    .then((components) => {
      components.forEach(({ cData, position, key, name }) => {
        switch (position) {
          case 'sidebar':
            app.component(key, cData[`${key}-entry`]);
            router.addRoute({
              path: `/${key}`,
              name: key,
              component: cData[`${key}-page`],
            });
            break;
          case 'board':
            app.component(name, cData[`${key}-index`]);
            if (
              app.config.globalProperties.$availKfBoards.indexOf(name) === -1
            ) {
              app.config.globalProperties.$availKfBoards.push(name);
            }
            break;
          default:
            app.component(key, cData[`${key}-index`]);
        }
      });
    })
    .then(() => {
      return useGlobalStore().setKfUIExtConfigs();
    })
    .then(() => {
      useGlobalStore().setKfConfigList();
      useGlobalStore().setKfExtConfigs();
    });
};
