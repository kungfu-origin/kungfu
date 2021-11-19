import { createApp } from 'vue';
import App from '@/App.vue';
import router from '@/router';
import store from '@/store';
import { Layout } from 'ant-design-vue';

import '@/assets/less/base.less';

createApp(App).use(store).use(router).use(Layout).mount('#app');
