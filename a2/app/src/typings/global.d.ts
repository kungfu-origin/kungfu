import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $registedKfUIComponents: string[];
        $bus: Subject;
        $useGlobalStore: StoreDefinition;
    }
}

export {};
