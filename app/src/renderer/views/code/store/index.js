import Vue from 'vue'
import Vuex from 'vuex'

import { KF_CONFIG_PATH } from '__gConfig/pathConfig';
import { readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { deepClone } from '__gUtils/busiUtils';

Vue.use(Vuex)

export default new Vuex.Store({

	state: {
        currentStrategy: '', //当前运行策略
        strategyList: [], //策略列表
        currentFile: {}, //文件树高亮
        entryFile: {}, //入口文件
        fileTree: {}, //文件树
        kfConfig: {} // kf 配置
    },

    mutations: {

        SET_CURRENT_STRATEGY (state, strategy) {
            state.currentStrategy = strategy;
        },

        SET_STRATEGY_LIST (state, strategyList) {
            state.strategyList = strategyList
        },

        SET_FILE_TREE (state, fileTree) {
            state.fileTree = fileTree
        },
        
        SET_FILE_NODE (state, { id, attr, val }) {
            const fileTree = state.fileTree;
            const node = deepClone(fileTree[id]);
            node[attr] = val;
            Vue.set(state.fileTree, id, node)
        },

        ADD_FILE_FOLDER_PENDING (state, { id, type }) {
            const targetChildren = state.fileTree[id].children;
            if(type == 'folder'){
                targetChildren['folder'].unshift('pending')
            }else{
                targetChildren['file'].unshift('pending')
            }
            Vue.set(state.fileTree[id], 'children', targetChildren)
            Vue.set(state.fileTree['pending'], 'parentId', id)
        },

        REMOVE_FILE_FOLDER_PENDING (state, { id, type }) {
            const targetChildren = state.fileTree[id].children;
            if(type == 'folder'){
                targetChildren['folder'].splice(targetChildren['folder'].indexOf('pending'), 1)
            }else{
                targetChildren['file'].splice(targetChildren['file'].indexOf('pending'), 1)
            }
            Vue.set(state.fileTree[id], 'children', targetChildren)
            Vue.set(state.fileTree['pending'], 'parentId', '')
        },


        SET_ENTRY_FILE (state, entryFile) {
            state.entryFile = entryFile
        },

        SET_CURRENT_FILE (state, file) {
            state.currentFile = file;
        },

        SET_KF_CONFIG (state, kfConfig) {
            Object.keys(kfConfig || {}).forEach(key => {
                Vue.set(state.kfConfig, key, kfConfig[key])
            })
        }
        
    },

    actions: {
        
        //设置当前策略
        setCurrentStrategy ({commit}, strategy ) {
            commit('SET_CURRENT_STRATEGY', strategy)
        },

        //common
        setStrategyList ({commit}, strategyList ) {
            commit('SET_STRATEGY_LIST', strategyList)
        },

        //策略编辑，设置当前文件
        setCurrentFile ({commit}, file ) {
            commit('SET_CURRENT_FILE', file)
        },

        //策略编辑，设置文件树
        setFileTree ({commit}, fileTree ) {
            commit('SET_FILE_TREE', fileTree)    
        },

        //策略编辑，设置文件节点
        setFileNode ({commit}, {id, attr, val} ) {
            commit('SET_FILE_NODE', {id, attr, val})
        },

        //策略编辑，添加文件或文件夹时，添加“pending”
        addFileFolderPending ({commit}, {id, type} ) {
            commit('ADD_FILE_FOLDER_PENDING', {id, type})
        },

        //策略编辑时，添加文件或文件夹时，删除“pending”
        removeFileFolderPending ({commit}, {id, type} ) {
            commit('REMOVE_FILE_FOLDER_PENDING', {id, type})
        },

        //标记入口文件
        setEntryFile ({commit}, entryFile ) {
            commit("SET_ENTRY_FILE", entryFile)
        },

        //common
        getKungfuConfig ({ dispatch }) {
            const kfConfig = readJsonSync(KF_CONFIG_PATH)
            dispatch('setKungfuConfig', kfConfig)
        },

        //common
        setKungfuConfig ({ commit }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            
        },

        //common
        setKungfuConfigByKeys ({ commit, state }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            outputJsonSync(KF_CONFIG_PATH, 
                {
                    ...state.kfConfig,
                    ...kfConfig
                }
            )
        },
    }
})