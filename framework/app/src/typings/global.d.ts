import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { Pm2ProcessStatusTypes } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { GlobalCategoryRegister } from '@renderer/assets/methods/uiExtraLocationUtils';
import { ProcessDescriptor } from 'ps-list';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $registedKfUIComponents: string[];
    $bus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
    $useGlobalStore: StoreDefinition;
    $globalCategoryRegister: GlobalCategoryRegister;
  }
}
declare global {
  interface Window {
    watcher: Watcher | null;
    kungfu: Kungfu;
  }

  namespace NodeJS {
    interface ProcessEnv {
      APP_TYPE: 'cli' | 'dzxy' | 'renderer' | 'component' | 'main';
      RENDERER_TYPE: 'app' | 'admin' | 'logview' | 'makeOrder' | 'codeEditor';
      RELOAD_AFTER_CRASHED: 'false' | 'true';
      ELECTRON_RUN_AS_NODE: 'false' | 'true';
    }
  }
}

export {};
