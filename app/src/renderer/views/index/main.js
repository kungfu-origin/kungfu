
/* eslint-disable */
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from '@/store';
import router from './routers';
import * as utils from '__gUtils/busiUtils'
import ElementUI from 'element-ui';
import Components from '@/assets/components'
import { remote } from 'electron'

import App from './App.vue';
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


const { startGetProcessStatus, startMaster, startLedger, startArchiveMakeTask } = require('__gUtils/processUtils');

startArchiveMakeTask((archiveStatus) => {
    window.archiveStatus = archiveStatus
})
.then(res => startMaster(false))
.finally(() => {
    startGetProcessStatus((processStatus) => {
        Vue.store.dispatch('setProcessStatus', processStatus)
    });

    utils.delayMiliSeconds(1000)
    .then(() => startLedger(false))
})

window.ELEC_WIN_MAP = new Set();

const currentWin = remote.getCurrentWindow()
currentWin.on('close', (e) => {
    Array.from(window.ELEC_WIN_MAP).forEach(winId => {
        const win = remote.BrowserWindow.fromId(winId)
        win && win.close && win.close()
    })
})