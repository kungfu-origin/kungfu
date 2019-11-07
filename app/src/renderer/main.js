
/* eslint-disable */
import Vue from 'vue';
import router from './routers';
import store from './store';
import Components from './assets/components'
import * as utils from '__gUtils/busiUtils'
import { Tag, Table, TableColumn, Col, Row, Input, InputNumber, DatePicker, Select, Option, Button, Tabs, TabPane, Card, Container, Header, Aside, Main, Footer, Dropdown, DropdownMenu, DropdownItem, Switch, MessageBox, Popover, Dialog, Loading, Radio, RadioGroup, Form, FormItem, Notification, Checkbox, Tooltip } from 'element-ui';
import moment from 'moment';
import App from './App.vue';
import { listProcessStatus, startMaster, startLedger } from '__gUtils/processUtils';
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';

Vue.use(Input)
Vue.use(InputNumber)
Vue.use(Tag)
Vue.use(Table)
Vue.use(TableColumn)
Vue.use(Col)
Vue.use(Row)
Vue.use(DatePicker)
Vue.use(Select)
Vue.use(Option)
Vue.use(Button)
Vue.use(Tabs)
Vue.use(TabPane)
Vue.use(Card)
Vue.use(Container)
Vue.use(Header)
Vue.use(Aside)
Vue.use(Main)
Vue.use(Footer)
Vue.use(Dropdown)
Vue.use(DropdownItem)
Vue.use(DropdownMenu)
Vue.use(Switch)
Vue.use(Popover)
Vue.use(Dialog)
Vue.use(Radio)
Vue.use(RadioGroup)
Vue.use(Form)
Vue.use(FormItem)
Vue.use(Loading)
Vue.use(Tooltip)
Vue.use(Checkbox)
Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.utils = Vue.prototype.$utils = utils
Vue.loading =  Vue.prototype.$loading = Loading.service;
Vue.confirm = Vue.prototype.$confirm = MessageBox.confirm;
Vue.notify =  Vue.prototype.$notify = Notification;

//tr的
Vue.use(Components)

//moment 格式
Vue.filter('moment', function (value, formatString) {
    formatString = formatString || 'YYYY-MM-DD HH:mm:ss';
    return moment(value).format(formatString); // value可以是普通日期 20170723
}); 

const currentPath = window.location.hash;
if(currentPath.indexOf('/code') === -1) {
    //循环获取processStatus
    var listProcessTimer;
    const startGetProcessStatus = () => {
        clearTimeout(listProcessTimer)
        listProcessStatus()
        .then(res => {
            const processStatus = Object.freeze(res);
            processStatus && Vue.store.dispatch('setProcessStatus', processStatus)
        })
        .catch(err => console.error(err))
        .finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
    }

    //start pm2 kungfu master
    process.env.ELECTRON_RUN_AS_NODE = true;

    startMaster(false)
    .catch(err => console.error(err))
    .finally(() => {
        startGetProcessStatus();
        utils.delayMiliSeconds(1000)
        .then(() => startLedger(false))
        .catch(err => console.error(err))
    })
}

new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)


// debug export
window.fileId = 0;
window.setTradingDay = (targetDay) => store.dispatch('setTradingDay', targetDay)
window.getTradingDay = () => store.state.BASE.tradingDay
window.store = store


