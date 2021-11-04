import Vue from 'vue';
import App from './App.vue';



new Vue({
    render: h => h(App)
}).$mount('#app', true)

const kungfu = require('@kungfu-trader/kungfu-core').kungfu();
const longfist = kungfu.longfist;
const { KF_RUNTIME_DIR } = require('@kungfu-trader/kungfu-uicc/config/pathConfig');
const path = require('path');

console.log(kungfu, KF_RUNTIME_DIR.replace(/\\/g, '\\\\'))


console.log(kungfu.ConfigStore(KF_RUNTIME_DIR))


