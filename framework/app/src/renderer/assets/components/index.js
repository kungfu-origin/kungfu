import { remote } from 'electron';

import TrNoData from './tr/TrNoData';
import TableHeader from './tr/TableHeader';
import TableHeaderItem from './tr/TableHeaderItem';
import TrStatus from './tr/TrStatus';
import TrBlinkNum from './tr/TrBlinkNum';
import TrTabTop from './tr/TrTabTop.vue';
import TrTabTopItem from './tr/TrTabTopItem.vue';
import TrFooterItem from './tr/TrFooterIterm.vue';
import TrMenu from './tr/TrMenu.vue';
import TrMenuItem from './tr/TrMenuItem.vue';
import TrDashboard from './tr/TrDashboard.vue';
import TrDashboardHeaderItem from './tr/TrDashboardHeaderItem.vue';
import TrTable from './tr/TrTable.vue';
import TrSearchInput from './tr/TrSearchInput.vue';
import TrSettingDashboard from './tr/TrSettingDashboard.vue';

import { openPage } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

const components = [
    TrNoData,
    TableHeader,
    TableHeaderItem,
    TrStatus,
    TrBlinkNum,
    TrTabTop,
    TrTabTopItem,
    TrFooterItem,
    TrMenu,
    TrMenuItem,
    TrDashboard,
    TrDashboardHeaderItem,
    TrTable,
    TrSearchInput,
    TrSettingDashboard
]

//导出
const { dialog } = require('electron').remote;
const saveFile = ({
    title, filters
}) => {
    return new Promise((resolve, reject) => {
        const options = {
            title: title || '保存文件',
            filters: [
                ...(filters || [
                    {name: 'csv', extensions: ['csv']}
                ])
            ]
        }
    
        dialog.showSaveDialog(options, (filename) => {
            resolve(filename)
        })
    })
}

//显示log
const showLog = (logPath) => {
    openPage(
        'showLog', 
        remote,
        {
            width: 600,
            height: 800,
            show: true
        }    
    ).then(({ win, curWinId }) => {
        win.webContents.send('show-log', {
            winId: curWinId,
            logPath
        });

    })
}


export default function (Vue) {
    components.map(component => {
        Vue.component(component.name, component)
    })

    Vue.saveFile = Vue.prototype.$saveFile = saveFile;
    Vue.showLog = Vue.prototype.$showLog = showLog;

    //message 换
    const Message = {
        error: function(message, duration){
            Vue.prototype.$notify({
                title: '错误',
                message: message,
                position: "top-right",
                type: 'error',
                duration: duration === 0 ? 0 : 4500
            })
        },
        warning: function(message){
            Vue.prototype.$notify({
                title: '警告',
                message: message,
                position: "top-right",
                type: 'warning'
            })
        },
        success: function(message){
            Vue.prototype.$notify({
                title: '成功',
                message: message,
                position: "top-right",
                type: 'success'
            })
        },
        start: function(message){
            Vue.prototype.$notify({
                title: '启动中',
                message: message,
                position: "top-right",
                type: 'info'
            })
        },
        info: function(message){
            Vue.prototype.$notify({
                title: '消息',
                message: message,
                position: "top-right",
                type: 'info'
            })
        }
    }
    Vue.message = Vue.prototype.$message = Message
}



