import {
  getUIComponents,
  loadExtScripts,
  loadExtComponents,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { App, defineAsyncComponent } from 'vue';

import { Router } from 'vue-router';
import { useGlobalStore } from './store/global';

import { useBoardFilter } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';

const { getBoard } = useBoardFilter();

export const useComponenets = (
  app: App<Element>,
  router: Router,
): Promise<void> => {
  const components = [
    'Pos',
    'PosGlobal',
    'Order',
    'Trade',
    'Td',
    'Md',
    'Strategy',
    'TradingTask',
    'MarketData',
    'OrderBook',
    'MakeOrder',
    'FutureArbitrage',
    'BlockTrade',
  ].filter((componentName) => getBoard(componentName, true, false));

  components.forEach((componentName) => {
    app.component(
      componentName,
      defineAsyncComponent(
        () =>
          import(
            `@kungfu-trader/kungfu-app/src/components/modules/${
              componentName[0].toLowerCase() + componentName.slice(1)
            }/${componentName}.vue`
          ),
      ),
    );
  });

  app.config.globalProperties.$availKfBoards = components.map(
    (componentName) => componentName,
  );

  return useGlobalStore()
    .setKfUIExtConfigs()
    .then((configs) => getUIComponents(configs))
    .then((components) => loadExtScripts(components, app))
    .then((components) => loadExtComponents(components, app, router))
    .then(() => {
      return useGlobalStore().setKfUIExtConfigs();
    })
    .then(() => {
      useGlobalStore().setKfConfigList();
      useGlobalStore().setKfExtConfigs();
    });
};
