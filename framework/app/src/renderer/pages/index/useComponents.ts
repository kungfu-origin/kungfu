import { getUIComponents } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { App, defineAsyncComponent } from 'vue';
import { Router } from 'vue-router';
import { useGlobalStore } from './store/global';
import path from 'path';
import fse from 'fs-extra';

export const useComponenets = (
  app: App<Element>,
  router: Router,
): Promise<void> => {
  app.component(
    '持仓',
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/pos/Pos.vue'),
    ),
  );

  app.component(
    '持仓汇总',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/posGlobal/PosGlobal.vue'
        ),
    ),
  );

  app.component(
    '委托记录',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/order/Order.vue'
        ),
    ),
  );

  app.component(
    '成交记录',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/trade/Trade.vue'
        ),
    ),
  );

  app.component(
    '交易账户',
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/td/Td.vue'),
    ),
  );

  app.component(
    '行情源',
    defineAsyncComponent(
      () =>
        import('@kungfu-trader/kungfu-app/src/components/modules/md/Md.vue'),
    ),
  );

  app.component(
    '策略进程',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/strategy/Strategy.vue'
        ),
    ),
  );

  app.component(
    '交易任务',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/tradingTask/TradingTask.vue'
        ),
    ),
  );

  app.component(
    '行情订阅',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/marketdata/MarketData.vue'
        ),
    ),
  );

  app.component(
    '深度行情',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/orderBook/OrderBook.vue'
        ),
    ),
  );

  app.component(
    '下单面板',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/makeOrder/MakeOrderDashboard.vue'
        ),
    ),
  );

  app.component(
    '套利指令',
    defineAsyncComponent(
      () =>
        import(
          '@kungfu-trader/kungfu-app/src/components/modules/futureArbitrage/FutureArbitrage.vue'
        ),
    ),
  );

  app.config.globalProperties.$availKfBoards = [
    '持仓',
    '持仓汇总',
    '委托记录',
    '成交记录',
    '交易账户',
    '行情源',
    '策略进程',
    '交易任务',
    '行情订阅',
    '深度行情',
    '下单面板',
    '套利指令',
  ];

  return useGlobalStore()
    .setKfUIExtConfigs()
    .then((configs) => getUIComponents(configs))
    .then((components) => {
      components.forEach(({ cData, position, key, name, extPath, script }) => {
        //load extension pure logic script
        const scriptPath = path.join(extPath, script);
        if (script && fse.pathExistsSync(scriptPath)) {
          app.use(global.require(scriptPath).default);
        }

        //registe different type ui components
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
