<template>
  <div class="file-tree">
    <div class="open-editor-folder">
      设置策略入口文件
    </div>
    <div class="file-tree-content">
      <div class="strategy-name">
        <span class="name">
          <span>sssss</span>
          （当前策略)
        </span>
        <span
          class="fr folder-oper"
          title="新建文件夹"
        >
          <i class="fl iconfont tr-folder1"></i>
        </span>
        <span
          class="fr folder-oper"
          title="新建文件"
        >
          <i class="fl iconfont tr-document1"></i>
        </span>
      </div>
      <div class="file-tree-body" v-if="strategyPath">
        <div v-for="file in fileTree">
            <FileNode
                v-if="file.root"
                :fileNode="file"
                :key="file.id"
                :id="file.id"
                type="folder"
            ></FileNode>
        </div>
      </div>
    </div>
  </div>
</template>
<script setup lang="ts">
import { defineProps, watch, ref, onMounted } from 'vue';
import path from 'path';
import { storeToRefs } from 'pinia';
import { message } from 'ant-design-vue';
import { buildFileObj } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { getTreeByFilePath } from '../../../assets/methods/codeUtils';
import { useCodeStore } from '../store/codeStore'

import FileNode from './FileNode.vue';
import { nextTick } from 'vue';
const store = useCodeStore();

const props = defineProps({
    strategy: Object
})
const { strategy } = props
const strategyPath = ref<string>('')
const strategyPathName = ref<string>('')

const { fileTree } = storeToRefs(useCodeStore());

watch(strategy as Code.Strategy, newStrategy => {
    getPath(newStrategy);
    initFileTree(newStrategy).then (fileTree => {
        const entryPath: string = newStrategy.strategy_path
        
        const currentFile: Code.FileData = (Object.values(fileTree || {}) as Code.FileData[]).filter(f => f.filePath === entryPath)[0]
        if (currentFile) {
            store.setEntryFile(currentFile)
            store.setCurrentFile(currentFile)
        }      
    })

})



//从prop内获取path
function getPath (strategy: Code.Strategy) {
    if (strategy && strategy.strategy_path) {
        //因为绑定策略时是文件，需要提取其父目录
        strategyPath.value = path.dirname(strategy.strategy_path);
        strategyPathName.value = path.basename(strategyPath.value);
    }
}
async function initFileTree(strategy) {
    if (!strategy.strategy_id || !strategy.strategy_path) return;
      //根文件夹得信息不像其他通过fs读取，而是直接从props的strategy中去取
    const rootId = window.fileId++;
    
    
    const rootFile: Code.FileData = buildFileObj({
        id: rootId,
        parentId: 0,
        isDir: true,
        name: strategyPathName.value,
        ext: '',
        filePath: strategyPath.value,
        children: {file: [], folder: []},
        stats: {},
        root: true,
        open: true,
    });
    //获取第一级文件树
    let ids, fileTree;
    try {
        const fileTreeData = await getTreeByFilePath(rootFile, fileTree);
        
        ids = fileTreeData.ids;
        fileTree = fileTreeData.fileTree
    } catch (err) {
        message.error(err);
    }

    // 处理根
    rootFile['children'] = ids;
    fileTree[rootId] = rootFile;
    // padding
    fileTree = bindFunctionalNode(fileTree);
    store.setFileTree(fileTree);
    store.setCurrentFile(rootFile)
    return fileTree
}

//增加 以pending 为key 的node，为addfile做准备
function bindFunctionalNode(fileTree) {
    fileTree['pending'] = {
        parentId: '',
    };
    return fileTree;
}

onMounted(() => {
    nextTick(() => {
    })
   
})
</script>

<style lang="less">
.file-tree {
    width: 100%;
}
</style>