import { createApp } from 'vue';
import App from '@renderer/index/App.vue';
import router from '@renderer/index/router';
import store from '@renderer/index/store';
import { Layout, Tabs, Button, Menu, Card } from 'ant-design-vue';

import '@renderer/assets/less/base.less';
import '@renderer/assets/less/coverAnt.less';

createApp(App)
    .use(store)
    .use(router)
    .use(Layout)
    .use(Tabs)
    .use(Button)
    .use(Menu)
    .use(Card)
    .mount('#app');
