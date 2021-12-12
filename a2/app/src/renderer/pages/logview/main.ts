import './setEnv';
import { createApp } from 'vue';
import App from '@renderer/pages/logview/App.vue';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { Button, Checkbox, Input, Layout, Spin } from 'ant-design-vue';

createApp(App)
    .use(Layout)
    .use(Button)
    .use(Checkbox)
    .use(Input)
    .use(Spin)
    .use(VueVirtualScroller)
    .mount('#app');
