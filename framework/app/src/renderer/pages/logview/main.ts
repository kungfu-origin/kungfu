import './setEnv';
import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/logview/App.vue';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { Button, Checkbox, Input, Layout, Spin } from 'ant-design-vue';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';

createApp(App)
  .use(Layout)
  .use(Button)
  .use(Checkbox)
  .use(Input)
  .use(Spin)
  .use(VueI18n)
  .use(VueVirtualScroller)
  .mount('#app');
