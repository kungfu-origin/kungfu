import { Subject } from 'rxjs';

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $registedKfUIComponents: string[];
        $bus: Subject;
    }
}

export {};
