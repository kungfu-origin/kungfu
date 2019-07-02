
import Vue from 'vue';
import * as ACCOUNT_API from '@/io/db/account';
import { debounce } from '@/assets/js/utils';

//保存选中的账户信息
export const setCurrentAccount = ({ commit }, account) => {
    commit('SET_CURRENT_ACCOUNT', account)
}

export const setOneMdTdState = ({ commit },  { id, stateData }) => {
    commit('SET_ONE_MD_TD_STATE',  { id, stateData })
}

//关闭状态删除某个状态
export const deleteOneMdTdState = ({ commit },  id) => {
    commit('DELETE_ONE_MD_TD_STATE',  id)
}

export const setAccountsAsset = ({ commit }, accountsAsset) => {
    commit('SET_ACCOUNTS_ASSET', accountsAsset)
}

export const setAccountAssetById = ({ commit },  { accountId, accountAsset }) => {
    commit('SET_ACCOUNT_ASSET',  { accountId, accountAsset })
}

export const setAccountList = ({ commit }, accountList) => {
    commit('SET_ACCOUNT_LIST', accountList)
}

//获取 accountList
export const getAccountList = ({ dispatch }) => {
    return new Promise((resolve, reject) => {
        ACCOUNT_API.getAccountList().then(res => {
            dispatch('setAccountList', res);
            resolve(res)
        })
    })
}



// //监听账户的一些信息，如td、md状态，账户资金信息
// var messageInfo = {};
// var debounceMessageError = debounce((gatewayName, message) => {
//     Vue.message.error(message);
//     messageInfo[gatewayName] = message;
// })

// export const buildGatewayNmsgListener = ({dispatch}, gatewayName) => {
//     if(!sub) return
//     const accountId = gatewayName.toAccountId()
//     //如果新的message，则报警
//     messageInfo[gatewayName] = ''
//     sub.on('data', buf => {
//         const data = JSON.parse(String(buf))
//         const dataMsgType = data.msg_type;
//         switch(dataMsgType){
//             case msgType.gatewayState: //监听td、md状态
//                 dispatch('setOneMdTdState', {name: gatewayName, oneState: Object.freeze(data.data)})
//                 if(data.data && data.data.message) {
//                     const message = data.data.message;
//                     if(messageInfo[gatewayName] !== message) debounceMessageError(gatewayName, data.data.message)
//                 }else {
//                     messageInfo[gatewayName] = '';
//                 }
//                 //更新md/td状态
//                 break;
//             case msgType.accountInfo: //监听资金
//                 dispatch('setAccountAssetById', {accountId, accountAsset: Object.freeze(data.data)})
//                 break
//         }
//     })
// }
