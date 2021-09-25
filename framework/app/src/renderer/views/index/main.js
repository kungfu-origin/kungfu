
/* eslint-disable */
import './errorCatch';
import './setKungfuParamsOnWindow';
import Vue from 'vue';
import path from 'path';
import fse from 'fs-extra';
import store from '@/store';
import router from './routers';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';
import { delayMilliSeconds, openVueWin, getTradingDate } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { removeJournal } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';
import { KF_HOME, KF_ADMIN_PASSWORD_CONFIG_PATH } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { watcher } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';
import { kungfu } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import ElementUI from 'element-ui';
import Components from '@/assets/components';

import App from './App.vue';
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';
import '@/assets/scss/makeOrder.scss';
import '@kungfu-trader/kungfu-uicc/io/http/index';

const remote = require("@electron/remote");

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

const { startGetProcessStatus, startMaster, startLedger, startDaemon, startArchiveMakeTask } = require('@kungfu-trader/kungfu-uicc/utils/processUtils');

if (!+process.env.RELOAD_AFTER_CRASHED) {
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
    
        delayMilliSeconds(1000)
            .then(() => startLedger(false))
            .catch(err => console.error(err.message))
    
        
        //保证ui watcher已经启动
        let timer = setInterval(() => {
            if (watcher.isLive() && watcher.isStarted() && watcher.isUsable()) {
                delayMilliSeconds(1000)
                    .then(() => { console.log("start daemon") })
                    .then(() => startDaemon())
                    .catch(err => console.error(err.message))
                clearInterval(timer);
            }
    
        }, 100)
    
    })
} else {
    // 崩溃后重开，跳过archive过程
    window.archiveStatus = true;
    startGetProcessStatus(res => {
        const { processStatus, processStatusWithDetail } = res;
        Vue.store.dispatch('setProcessStatus', processStatus)
        Vue.store.dispatch('setProcessStatusWithDetail', processStatusWithDetail)
    });
}

function beforeAll () {
    if (process.env.NODE_ENV !== 'development') {
        const clearJournalDateFromLocal = localStorage.getItem("clearJournalTradingDate");
        const currentTradingDate = getTradingDate();
        console.log("Lastest Clear Journal Trading Date: ", clearJournalDateFromLocal);

        if (currentTradingDate !== clearJournalDateFromLocal) {
            localStorage.setItem('clearJournalTradingDate', currentTradingDate);
            console.log("Clear Journal Trading Date: ", currentTradingDate);
            return removeJournal(KF_HOME);
        } else {
            return Promise.resolve(true);
        }
    } else {
        return Promise.resolve(true);
    }
}


//admin manager
var adminWin = null;
window.admin = {
    login (password) {
        const hashedPassword = kungfu.formatStringToHashHex(password.toString());
        const rightPassword = fse.readJsonSync(KF_ADMIN_PASSWORD_CONFIG_PATH);
    
        if (hashedPassword != rightPassword.password || '') {
            console.error("管理员密码错误！")
            return;
        }
    
        //防止重开
        if (adminWin) {
            adminWin.focus && adminWin.focus();
            return;
        }
        
        openVueWin(
            "admin",
            "/",
            remote
        ).then(win => {
            logger.info("Admin login")
            adminWin = win;
            adminWin.on('close', () => {
                logger.info("Admin logout")
                adminWin = null;
            })
        })
        console.log("管理员系统打开成功！")
        return
    },

    resetPassword (oldpassword, newpassword) {
        const targetJSONPath = path.resolve(KF_ADMIN_PASSWORD_CONFIG_PATH);
        const oldHashedPassword = kungfu.formatStringToHashHex(oldpassword.toString());
        const rightOldPassword = fse.readJsonSync(targetJSONPath);
        
        if (oldHashedPassword != rightOldPassword.password || '') {
            console.error("管理员旧密码错误！")
            console.error("更新密码失败！")
            return;
        }

        const newHashedPassword = kungfu.formatStringToHashHex(newpassword.toString());
        fse.writeJSONSync(targetJSONPath, {
            password: newHashedPassword
        })
        console.log("管理员新密码设置成功！")
        console.log("请通过admin.login方法登录")
        return;
    }
};


if (module.hot) {    
    module.hot.accept();
}
