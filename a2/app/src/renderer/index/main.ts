import { createApp } from 'vue';
import App from '@renderer/index/App.vue';
import router from '@renderer/index/router';
import store from '@renderer/index/store';
import { Layout, Tabs, Button } from 'ant-design-vue';

import '@renderer/index/assets/less/base.less';

createApp(App)
    .use(store)
    .use(router)
    .use(Layout)
    .use(Tabs)
    .use(Button)
    .mount('#app');
