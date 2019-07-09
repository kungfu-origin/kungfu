import * as STRATEGY_API from '__io/db/strategy';
import { deleteStrat } from '__io/actions/strategy';

//设置当前策略
export const setCurrentStrategy = ({commit}, strategy) => {
    commit('SET_CURRENT_STRATEGY', strategy)
}

//设置当前策略列表
export const setStrategyList = ({commit}, strategyList) => {
    commit('SET_STRATEGY_LIST', strategyList)
}

//策略编辑，设置当前文件
export const setCurrentFile = ({commit}, file) => {
    commit('SET_CURRENT_FILE', file)
}

//策略编辑，设置文件树
export const setFileTree = ({commit}, fileTree) => {
    commit('SET_FILE_TREE', fileTree)    
}

//策略编辑，设置文件节点
export const setFileNode = ({commit}, {id, attr, val}) => {
    commit('SET_FILE_NODE', {id, attr, val})
}

//策略编辑，添加文件或文件夹时，添加“pending”
export const addFileFolderPending = ({commit}, {id, type}) => {
    commit('ADD_FILE_FOLDER_PENDING', {id, type})
}

//策略编辑时，添加文件或文件夹时，删除“pending”
export const removeFileFolderPending = ({commit}, {id, type}) => {
    commit('REMOVE_FILE_FOLDER_PENDING', {id, type})

}

//更新straetegyList
export const getStrategyList = ({dispatch}) => {
    return new Promise(resolve => {
        STRATEGY_API.getStrategyList().then((res) => {
            dispatch('setStrategyList', res);
            resolve(res)
        })
    })
}

//标记入口文件
export const setEntryFile = ({commit}, entryFile) => {
    commit("SET_ENTRY_FILE", entryFile)
}

//删除策略
export const deleteStrategy = ({dispatch}, strategyId) => {
    return deleteStrat(strategyId).then(() => dispatch('getStrategyList'))
}
