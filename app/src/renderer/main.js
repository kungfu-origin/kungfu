/* eslint-disable */
import Vue from 'vue';
import router from './routers';
import store from './store';
import Components from './assets/components'
import * as utils from '__gUtils/busiUtils'
import {Tag, Table, TableColumn, Col, Row, Input, InputNumber, DatePicker, Select, Option, Button, Tabs, TabPane, Card, Container, Header, Aside, Main, Footer, Dropdown, DropdownMenu, DropdownItem, Switch, MessageBox, Popover, Dialog, Loading, Radio, RadioGroup, Form, FormItem, Notification, Checkbox, Tooltip} from 'element-ui';
import moment from 'moment';
import App from './App.vue';
import { listProcessStatus, startMaster, startWatcher } from '__gUtils/processUtils';
import { ipcRenderer } from 'electron'
import '@/assets/iconfont/iconfont.js';
import '@/assets/iconfont/iconfont.css';

//element
Vue.use(Tag)
Vue.use(Table)
Vue.use(TableColumn)
Vue.use(Col)
Vue.use(Row)
Vue.use(Input)
Vue.use(InputNumber)
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


//循环获取processStatus
var listProcessTimer;
export const startGetProcessStatus = () => {
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
    startWatcher(false)
    .catch(err => console.error(err))
    .finally(() => startGetProcessStatus())
})


/* eslint-disable no-new */
new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)



//自动更新逻辑
startAutoUpdate()

function startAutoUpdate(){
    if(window.location.href.split('#')[1].indexOf('code') !== -1) return;
    //remove只能移除单个事件，单独封装removeAll移除所有事件
    ipcRenderer.removeAllListeners('message')
    ipcRenderer.send('checkForUpdate')
    ipcRenderer.on("message", (event, text) => console.log(text));
    //注意：“downloadProgress”事件可能存在无法触发的问题，只需要限制一下下载网速就好了
}

window.fileId = 0;