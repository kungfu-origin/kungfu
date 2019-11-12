import TrNoData from './tr/TrNoData';
import TableHeader from './tr/TableHeader';
import TableHeaderItem from './tr/TableHeaderItem';
import TrStatus from './tr/TrStatus';
import TrTabTop from './tr/TrTabTop.vue';
import TrTabTopItem from './tr/TrTabTopItem.vue';
import TrFooterItem from './tr/TrFooterIterm.vue';
import TrMenu from './tr/TrMenu.vue';
import TrMenuItem from './tr/TrMenuItem.vue';
import TrPnl from './tr/TrPnl.vue';
import TrDashboard from './tr/TrDashboard.vue';
import TrDashboardHeaderItem from './tr/TrDashboardHeaderItem.vue';
import TrTable from './tr/TrTable.vue';
import TrSearchInput from './tr/TrSearchInput.vue';
import TrSettingDashboard from './tr/TrSettingDashboard.vue';

import MainContent from '@/components/Layout/MainContent';
import { buildTask } from '__gUtils/busiUtils';

const components = [
    TrNoData,
    TableHeader,
    TableHeaderItem,
    MainContent,
    TrStatus,
    TrTabTop,
    TrTabTopItem,
    TrFooterItem,
    TrMenu,
    TrMenuItem,
    TrDashboard,
    TrDashboardHeaderItem,
    TrPnl,
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
const BrowserWindow = require('electron').remote.BrowserWindow;
const showLog = (logPath) => {
    buildTask(
        'showLog', 
        BrowserWindow.getFocusedWindow(), 
        BrowserWindow,
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
            Vue.notify({
                title: '错误',
                message: message,
                position: "bottom-right",
                type: 'error',
                duration: duration === 0 ? 0 : 4500
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
        },
        info: function(message){
            Vue.notify({
                title: '消息',
                message: message,
                position: "bottom-right",
                type: 'info'
            })
        }
    }
    Vue.message = Vue.prototype.$message = Message
}



