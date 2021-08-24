
/* eslint-disable */
import './errorCatch';
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from '@/store';
import router from './routers';
import * as utils from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { removeJournal } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';
import { KF_HOME } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { watcher } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';
import ElementUI from 'element-ui';
import Components from '@/assets/components';

import App from './App.vue';
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';
import '@/assets/scss/makeOrder.scss';
import moment from 'moment';
import '@kungfu-trader/kungfu-uicc/io/http/index';

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


const { startGetProcessStatus, startMaster, startLedger, startDaemon, startArchiveMakeTask, _pm2 } = require('@kungfu-trader/kungfu-uicc/utils/processUtils');


beforeAll()
.then(() => {
    return startArchiveMakeTask((archiveStatus) => {
        window.archiveStatus = archiveStatus
    })
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

    
    //保证ui watcher已经启动
    let timer = setInterval(() => {
        if (watcher.isLive() && watcher.isStarted() && watcher.isUsable()) {
            utils.delayMiliSeconds(1000)
                .then(() => startDaemon())
                .catch(err => console.error(err.message))
            clearInterval(timer);
        }

    }, 100)

})

window.ELEC_WIN_MAP = new Set();
window.pm2 = _pm2;


function beforeAll () {
    if (process.env.NODE_ENV !== 'development') {
        const clearJournalDate = localStorage.getItem('clearJournalDate');
        const today = moment().format('YYYY-MM-DD');
        console.log( localStorage.getItem('clearJournalDate'), today)
        
        if (clearJournalDate !== today) {
            localStorage.setItem('clearJournalDate', today);
            console.log( localStorage.getItem('clearJournalDate'), today)
            return removeJournal(KF_HOME);
        } else {
            return Promise.resolve(true);
        }
    } else {
        return Promise.resolve(true);
    }
}