import { createApp } from 'vue';
import App from '@renderer/pages/index/App.vue';
import router from '@renderer/pages/index/router';
import store from '@renderer/pages/index/store';
import { Layout, Tabs, Button, Menu, Card } from 'ant-design-vue';

createApp(App)
    .use(store)
    .use(router)
    .use(Layout)
    .use(Tabs)
    .use(Button)
    .use(Menu)
    .use(Card)
    .mount('#app');
