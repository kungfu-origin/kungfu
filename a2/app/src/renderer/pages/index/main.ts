import { createApp, Component } from 'vue';
import { Subject } from 'rxjs';
import App from '@renderer/pages/index/App.vue';
import router from '@renderer/pages/index/router';
import store from '@renderer/pages/index/store';
import {
    Layout,
    Tabs,
    Button,
    Menu,
    Card,
    Input,
    Table,
    Switch,
    ConfigProvider,
    Modal,
    Radio,
    Tag,
    Form,
    InputNumber,
    Select,
} from 'ant-design-vue';

import '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { getUIComponents } from '@renderer/assets/methods/uiUtils';
import { useGlobalStore } from '@renderer/pages/index/store/global';

const app = createApp(App);

app.use(store);

const uics = getUIComponents();
Object.keys(uics).forEach((key) => {
    app.component(key, uics[key] as Component);
});

//this sort ensure $useGlobalStore can be get in mounted callback
app.config.globalProperties.$registedKfUIComponents = Object.keys(uics);
app.config.globalProperties.$bus = new Subject();
app.config.globalProperties.$useGlobalStore = useGlobalStore;

app.use(router)
    .use(Layout)
    .use(Tabs)
    .use(Button)
    .use(Menu)
    .use(Card)
    .use(Input)
    .use(Table)
    .use(Switch)
    .use(ConfigProvider)
    .use(Modal)
    .use(Radio)
    .use(Tag)
    .use(InputNumber)
    .use(Select)
    .use(Form);

app.mount('#app');
