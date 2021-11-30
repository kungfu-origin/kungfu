import { Subject } from 'rxjs';

export {};

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $registedKfUIComponents: string[];
        $bus: Subject;
    }
}

