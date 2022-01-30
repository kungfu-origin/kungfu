import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { GlobalCategoryRegister } from '@renderer/assets/methods/uiExtraLocationUtils';

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
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
    }
  }
}

export {};
