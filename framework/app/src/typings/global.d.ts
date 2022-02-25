import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { GlobalCategoryRegister } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $globalBus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
    $useGlobalStore: StoreDefinition;
    $globalCategoryRegister: GlobalCategoryRegister;
    $availKfBoards: string[];
  }
}

declare module 'worker-loader!*' {
  class WebpackWorker extends Worker {
    constructor();
  }

  export default WebpackWorker;
}

declare global {
  interface Window {
    watcher: Watcher | null;
    kungfu: Kungfu;
    workers: Record<string, WebpackWorker>;
  }

  namespace NodeJS {
    interface ProcessEnv {
      APP_TYPE: 'cli' | 'dzxy' | 'renderer' | 'component' | 'main';
      RENDERER_TYPE: 'app' | 'admin' | 'logview' | 'makeOrder' | 'codeEditor';
      RELOAD_AFTER_CRASHED: 'false' | 'true';
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
    }
  }
}

export {};
