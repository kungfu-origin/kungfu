
/* eslint-disable */
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from './store';
import router from './routers';
import * as utils from '__gUtils/busiUtils'
import ElementUI from 'element-ui';
import Components from '@/assets/components'
import App from './App.vue';
import electron from 'electron';

import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';

Vue.use(ElementUI)

Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.utils = Vue.prototype.$utils = utils

//tr 组件
Vue.use(Components)


new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)


//kungfu master 启动流程
const { startGetProcessStatus } = require('__gUtils/processUtils');
startGetProcessStatus((processStatus) => {
    Vue.store.dispatch('setProcessStatus', processStatus)
});

window.electron = electron

