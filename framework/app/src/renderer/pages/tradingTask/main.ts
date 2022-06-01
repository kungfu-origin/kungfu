import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/TradingTask/App.vue';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { Layout, Card, Empty } from 'ant-design-vue';

const app = createApp(App);

app.use(Layout).use(Card).use(Empty);

app.use(VueI18n);
app.mount('#app');
