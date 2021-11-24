import { createApp } from 'vue';
import App from '@/index/App.vue';
import router from '@/index/router';
import store from '@/index/store';
import { Layout, Tabs, Button } from 'ant-design-vue';

import '@/index/assets/less/base.less';

createApp(App)
    .use(store)
    .use(router)
    .use(Layout)
    .use(Tabs)
    .use(Button)
    .mount('#app');
