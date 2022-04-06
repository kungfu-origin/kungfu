import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/code/App.vue';
import store from '../index/store';
import { Input, Button, Layout, Select } from 'ant-design-vue';
import 'monaco-editor/min/vs/editor/editor.main.css';
const app = createApp(App);

app.component('ComFileNode');
app.use(Input).use(Button).use(Layout).use(Select).use(store);
app.mount('#app');
window.fileId = 0;
