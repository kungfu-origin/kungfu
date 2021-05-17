
/* eslint-disable */
import './errorCatch';
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from '@/store';
import router from './routers';
import * as utils from '__gUtils/busiUtils';
import { removeJournal } from '__gUtils/fileUtils';
import { KF_HOME } from '__gConfig/pathConfig';
import { watcher } from '__io/kungfu/watcher';
import ElementUI from 'element-ui';
import Components from '@/assets/components';

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


const { startGetProcessStatus, startMaster, startLedger, startDaemon, startArchiveMakeTask, _pm2 } = require('__gUtils/processUtils');


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
        return removeJournal(KF_HOME)
    } else {
        return Promise.resolve(true)
    }
}