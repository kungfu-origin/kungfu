import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/code/App.vue';
import store from '../index/store';
import { Input, Button, Layout, Dropdown, Menu } from 'ant-design-vue';
import 'monaco-editor/min/vs/editor/editor.main.css';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';

const app = createApp(App);

app.component('ComFileNode');

app
  .use(Input)
  .use(Button)
  .use(Layout)
  .use(Dropdown)
  .use(VueI18n)
  .use(Menu)
  .use(store);

app.mount('#app');

window.fileId = 0;
