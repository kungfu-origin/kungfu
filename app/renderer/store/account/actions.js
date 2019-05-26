
import Vue from 'vue';
import {closeGlobalGatewayNanomsg, refreshGlobalGatewayNanomsg} from '@/io/nano/buildNmsg'
import * as msgType from '@/io/nano/msgType'
import * as ACCOUNT_API from '@/io/account';
import {startTd, startMd, deleteProcess} from '__gUtils/processUtils'
import {debounce} from '@/assets/js/utils'



//保存选中的账户信息
export const setCurrentAccount = ({commit}, account) => {
    commit('SET_CURRENT_ACCOUNT', account)
}

export const setMdTdState = ({commit}, mdTdState) => {
    commit('SET_MD_TD_STATE', mdTdState)
}

export const setOneMdTdState = ({commit},  {name, oneState}) => {
    commit('SET_ONE_MD_TD_STATE',  {name, oneState})
}

//关闭状态删除某个状态
export const deleteOneMdTdState = ({commit},  name) => {
    commit('DELETE_ONE_MD_TD_STATE',  name)
    return true
}

export const setAccountsAsset = ({commit}, accountsAsset) => {
    commit('SET_ACCOUNTS_ASSET', accountsAsset)
}

export const setAccountAssetById = ({commit},  {accountId, accountAsset}) => {
    
    commit('SET_ACCOUNT_ASSET',  {accountId, accountAsset})
}

export const setAccountList = ({commit}, accountList) => {
    commit('SET_ACCOUNT_LIST', accountList)
}

//获取 accountList
export const getAccountList = ({dispatch}) => {
    return new Promise((resolve,reject) => {
        ACCOUNT_API.getAccountList().then(res => {
            dispatch('setAccountList', res);
            resolve(res)
        }).catch(err => {
            Vue.message.error(err.message || '操作失败！')
        })
    })
}



//监听账户的一些信息，如td、md状态，账户资金信息
var messageInfo = {};
var debounceMessageError = debounce((gatewayName, message) => {
    Vue.message.error(message);
    messageInfo[gatewayName] = message;
})

export const buildGatewayNmsgListener = ({dispatch}, gatewayName) => {
    const sub = refreshGlobalGatewayNanomsg(gatewayName)
    if(!sub) return
    const accountId = gatewayName.toAccountId()
    //如果新的message，则报警
    messageInfo[gatewayName] = ''
    sub.on('data', buf => {
        const data = JSON.parse(String(buf))
        const dataMsgType = data.msg_type;
        switch(dataMsgType){
            case msgType.gatewayState: //监听td、md状态
                dispatch('setOneMdTdState', {name: gatewayName, oneState: Object.freeze(data.data)})
                if(data.data && data.data.message) {
                    const message = data.data.message;
                    if(messageInfo[gatewayName] !== message) debounceMessageError(gatewayName, data.data.message)
                }else {
                    messageInfo[gatewayName] = '';
                }
                //更新md/td状态
                break;
            case msgType.accountInfo: //监听资金
                dispatch('setAccountAssetById', {accountId, accountAsset: Object.freeze(data.data)})
                break
        }
    })
}


//起停td
export const switchTd = ({dispatch}, {account, value}) => {
    const {account_id, source_name, config} = account
    const tdProcessId = `td_${account_id}`
    if(!value){
        return deleteProcess(tdProcessId)
        .then(() => dispatch('deleteOneMdTdState', tdProcessId))
        .then(() => closeGlobalGatewayNanomsg(tdProcessId))
        .then(() => Vue.message.success('操作成功！'))        
        .catch(err => Vue.message.error(err.message || '操作失败！'))
    }

    //改变数据库表内容，添加或修改
    return dispatch('setTasksDB', {name: tdProcessId, type: 'td', config})
    .then(() => dispatch('getTasks'))//重新获取数据
    .then(() => dispatch('setOneMdTdState', {name: tdProcessId, oneState: Object.freeze({})}))
    .then(() => dispatch('buildGatewayNmsgListener', tdProcessId))    
    .then(() => startTd(source_name, tdProcessId)) //开启td,pm2
    .then(() => Vue.message.start('正在启动...'))       
    .catch(err => Vue.message.error(err.message || '操作失败！'))
}

//起停md
export const switchMd = ({dispatch}, {account, value}) => {
    const {source_name, config} = account;
    const mdProcessId = `md_${source_name}`
    if(!value){
        return deleteProcess(mdProcessId)
        .then(() => dispatch('deleteOneMdTdState', mdProcessId))
        .then(() => closeGlobalGatewayNanomsg(mdProcessId))
        .then(() => Vue.message.success('操作成功！'))        
        .catch(err => Vue.message.error(err.message || '操作失败！'))
    }

    //改变数据库表内容，添加或修改
    return dispatch('setTasksDB', {name: mdProcessId, type: 'md', config})
    .then(() => dispatch('getTasks'))//重新获取数据
    .then(() => dispatch('setOneMdTdState', {name: mdProcessId, oneState: Object.freeze({})}))
    .then(() => dispatch('buildGatewayNmsgListener', mdProcessId))    
    .then(() => startMd(source_name, mdProcessId)) //开启td,pm2
    .then(() => Vue.message.start('正在启动...'))       
    .catch(err => Vue.message.error(err.message || '操作失败！'))          
}