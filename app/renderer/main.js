/* eslint-disable */
import Vue from 'vue';
import router from './routers';
import store from './store';
import Components from './assets/components'
import * as utils from './assets/js/utils'
import {Tag, Table, TableColumn, Col, Row, Input, InputNumber, DatePicker, Select, Option, Button, Tabs, TabPane, Card, Container, Header, Aside, Main, Footer, Dropdown, DropdownMenu, DropdownItem, Switch, MessageBox, Popover, Dialog, Loading, Radio, RadioGroup, Form, FormItem, Notification, Checkbox, Tooltip} from 'element-ui';
import moment from 'moment';
import App from './App.vue';
import {EVENT_BUS} from './io/event-bus'
import {listProcessStatus} from '__gUtils/processUtils';

import './assets/iconfont/iconfont.css';
import './assets/css/font-awesome.min.css';


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
Vue.use(Components)
Vue.use(Tooltip)
Vue.use(Checkbox)
Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.utils = Vue.prototype.$utils = utils
Vue.loading =  Vue.prototype.$loading = Loading.service;
Vue.confirm = Vue.prototype.$confirm = MessageBox.confirm;
Vue.notify =  Vue.prototype.$notify = Notification;

//message 换
const Message = {
    error: function(message){
        Vue.notify({
            title: '错误',
            message: message,
            position: "bottom-right",
            type: 'error'
        })
    },
    warning: function(message){
        Vue.notify({
            title: '警告',
            message: message,
            position: "bottom-right",
            type: 'warning'
        })
    },
    success: function(message){
        Vue.notify({
            title: '成功',
            message: message,
            position: "bottom-right",
            type: 'success'
        })
    },

    start: function(message){
        Vue.notify({
            title: '启动中',
            message: message,
            position: "bottom-right",
            type: 'info'
        })
    }
}

Vue.message = Vue.prototype.$message = Message

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
        console.log(JSON.stringify(res))
        res && EVENT_BUS.$emit('update-process-status', Object.freeze(res))
    })
    .catch(err => console.error(err))
    .finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
}

//start pm2 kungfu engine
process.env.ELECTRON_RUN_AS_NODE = true;
const {startPageEngine, startCalendarEngine} = require('__gUtils/processUtils');
startPageEngine(false)
.then(() => startCalendarEngine(false))
.catch(err => console.error(err))
.finally(() => {
    startGetProcessStatus()
})

/* eslint-disable no-new */
new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)
