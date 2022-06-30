import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { GlobalCategoryRegister } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $globalBus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
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
    fileId: number;
    testCase: Record<string, any>;
  }

  namespace NodeJS {
    interface ProcessEnv {
      LANG_ENV: 'zh-CN' | 'en-US' | 'zh-HK' | undefined;
      APP_TYPE: 'cli' | 'renderer' | 'component' | 'daemon' | 'main';
      DAEMON_TYPE: string;
      RENDERER_TYPE:
        | 'app'
        | 'admin'
        | 'logview'
        | 'makeOrder'
        | 'codeEditor'
        | 'dzxy';
      EXTENSION_DIRS: string;
      KFC_DIR: string;
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
    }
  }
}

export {};
