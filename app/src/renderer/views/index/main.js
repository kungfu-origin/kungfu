
/* eslint-disable */
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from '@/store';
import router from './routers';
import * as utils from '__gUtils/busiUtils'
import ElementUI from 'element-ui';
import Components from '@/assets/components'
import moment from 'moment';
import App from './App.vue';
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';

Vue.use(ElementUI)

Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.utils = Vue.prototype.$utils = utils

//tr 组件
Vue.use(Components)

//moment 格式
Vue.filter('moment', function (value, formatString) {
    formatString = formatString || 'YYYY-MM-DD HH:mm:ss';
    return moment(value).format(formatString); // value可以是普通日期 20170723
}); 

new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)


const { startGetProcessStatus, startMaster, startLedger } = require('__gUtils/processUtils');
//kungfu master 启动流程
startMaster(false)
    .finally(() => {
        startGetProcessStatus((processStatus) => {
            Vue.store.dispatch('setProcessStatus', processStatus)
        });

        utils.delayMiliSeconds(1000)
        .then(() => startLedger(false))
    })


