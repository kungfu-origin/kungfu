import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { GlobalCategoryRegister } from '@kungfu-trader/kungfu-js-api/utils/extraLocationUtils';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $globalBus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
    $globalCategoryRegister: GlobalCategoryRegister;
    $availKfBoards: string[];
    _: any; //vue3 shit
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
      DAEMON_ID: string;
      RENDERER_ID: string;
      EXTENSION_DIRS: string;
      KFC_DIR: string;
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
    }
  }
}

export {};
