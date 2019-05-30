import Vue from 'vue';
import {deepClone} from '@/assets/js/utils'

export const SET_CURRENT_STRATEGY = (state, strategy) => {
    state.currentStrategy = strategy;
}

export const SET_STRATEGY_LIST = (state, strategyList) => {
    state.strategyList = strategyList
}


export const SET_CURRENT_FILE = (state, file) => {
    state.currentFile = file;
}

export const SET_FILE_TREE = (state, fileTree) => {
    state.fileTree = fileTree
}

export const SET_FILE_NODE = (state, {id, attr, val}) => {
    const fileTree = state.fileTree;
    const node = deepClone(fileTree[id]);
    node[attr] = val;
    Vue.set(state.fileTree, id, node)
}

export const ADD_FILE_FOLDER_PENDING = (state, {id, type}) => {
    const targetChildren = state.fileTree[id].children;
    if(type == 'folder'){
        targetChildren['folder'].unshift('pending')
    }else{
        targetChildren['file'].unshift('pending')
    }
    Vue.set(state.fileTree[id], 'children', targetChildren)
    Vue.set(state.fileTree['pending'], 'parentId', id)
}

export const REMOVE_FILE_FOLDER_PENDING = (state, {id, type}) => {
    const targetChildren = state.fileTree[id].children;
    if(type == 'folder'){
        targetChildren['folder'].splice(targetChildren['folder'].indexOf('pending'), 1)
    }else{
        targetChildren['file'].splice(targetChildren['file'].indexOf('pending'), 1)
    }
    Vue.set(state.fileTree[id], 'children', targetChildren)
    Vue.set(state.fileTree['pending'], 'parentId', '')
}

export const SET_ENTRY_FILE = (state, entryFile) => {
    state.entryFile = entryFile
}