
/* eslint-disable */
import './errorCatch';
import Vue from 'vue';
import path from 'path';
import './setKungfuParamsOnWindow';
import store from '@/store';
import router from './routers';
import * as utils from '__gUtils/busiUtils';
import ElementUI from 'element-ui';
import Components from '@/assets/components';

import { switchTask } from '__io/actions/base';
import { KF_DATASET_QUOTE_DIR } from '__gConfig/pathConfig';

import App from './App.vue';
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';
import '@/assets/scss/makeOrder.scss';

Vue.use(ElementUI)
Vue.use(Components)

Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.bus = Vue.prototype.$bus = new Vue();


new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)


const { startGetProcessStatus, startMaster, startLedger, startArchiveMakeTask, _pm2 } = require('__gUtils/processUtils');

startArchiveMakeTask((archiveStatus) => {
    window.archiveStatus = archiveStatus
})
.then(() => startMaster(false))
.catch(err => console.error(err.message))
.finally(() => {
    startGetProcessStatus(res => {
        const { processStatus, processStatusWithDetail } = res;
        Vue.store.dispatch('setProcessStatus', processStatus)
        Vue.store.dispatch('setProcessStatusWithDetail', processStatusWithDetail)
    });

    utils.delayMiliSeconds(1000)
        .then(() => startLedger(false))
        .catch(err => console.error(err.message))
})

window.ELEC_WIN_MAP = new Set();
window.pm2 = _pm2;

