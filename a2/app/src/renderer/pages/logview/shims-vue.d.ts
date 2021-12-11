/* eslint-disable */
declare module '*.vue' {
    import type { DefineComponent } from 'vue';
    const component: DefineComponent<{}, {}, any>;
    export default component;
}

declare module 'vue-virtual-scroller' {
    import { App } from 'vue';
    export default {
        install(app: App): void;,
    };
}
