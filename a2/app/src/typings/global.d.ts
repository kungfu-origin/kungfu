import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { Pm2ProcessStatusTypes } from '@kungfu-trader/kungfu-js-api/utils/processUtils';

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $registedKfUIComponents: string[];
        $bus: Subject;
        $useGlobalStore: StoreDefinition;
    }
}

declare global {
    interface Window {
        archiveStatus: Pm2ProcessStatusTypes;
    }

    namespace NodeJS {
        interface ProcessEnv {
            RENDERER_TYPE: 'app' | 'admin' | 'makeOrder' | 'codeEditor';
            RELOAD_AFTER_CRASHED: 'false' | 'true';
            ELECTRON_RUN_AS_NODE: 'false' | 'true';
        }
    }
}

export {};
