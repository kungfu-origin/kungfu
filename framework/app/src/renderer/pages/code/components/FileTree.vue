<template>
  <div class="file-tree">
    <div class="open-editor-folder" @click="handleBindStrategyFolder">
      设置策略入口文件
    </div>
    <div class="file-tree-content">
      <div class="strategy-name">
        <span class="name">
          <span v-if="strategy">{{ strategy.strategy_id }}</span>
          （当前策略)
        </span>
        <span
          class="fr folder-oper"
          title="新建文件夹"
          v-if="strategyPath"
          @click="handleAddFolder"
        >
          <i class="fl iconfont tr-folder1"></i>
        </span>
        <span
          class="fr folder-oper"
          title="新建文件"
          v-if="strategyPath"
          @click="handleAddFile"
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

<script lang="ts">
export default {
    emits: ['updateStrategy']
}
</script>
<script setup lang="ts">
import { defineProps, watch, ref, onMounted, getCurrentInstance, ComponentInternalInstance } from 'vue';
import path from 'path';
import { storeToRefs } from 'pinia';
import { dialog } from '@electron/remote';
import { message } from 'ant-design-vue';
import { buildFileObj } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { getTreeByFilePath } from '../../../assets/methods/codeUtils';
import { useCodeStore } from '../store/codeStore'
import FileNode from './FileNode.vue';
import { nextTick } from 'vue';
// import { ipcEmitDataByName } from '../emitter';
import { openFolder } from '../../../assets/methods/codeUtils';
const store = useCodeStore();

const props = defineProps({
    strategy: Object
})
const { strategy } = props
const strategyPath = ref<string>('')
const strategyPathName = ref<string>('')
const { proxy } = getCurrentInstance() as ComponentInternalInstance
const { fileTree, currentFile } = storeToRefs(useCodeStore());

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
 //绑定策略
function handleBindStrategyFolder() {
    dialog.showOpenDialog(
        {
            properties: ['openFile'],
        },
    ).then (strategyPath => {
            if (!strategyPath || !strategyPath.filePaths[0]) return;
            if (!strategy?.strategy_id) return;
            bindStrategyPath(strategyPath.filePaths);
    });
}

//bind data中path 与 sqlite中path
async function bindStrategyPath(strategyPath) {
    // await ipcEmitDataByName('updateStrategyPath', {
    //     strategyId: strategy?.strategy_id,
    //     strategyPath: strategyPath[0],
    // });
    if (strategy && strategy.strategy_id) {
        
        message.success(
            `策略${strategy.strategy_id}文件路径修改成功！`,
        );
        //每次更新path，需要通知root组件更新stratgy
        proxy?.$emit('updateStrategy', strategyPath);
    }
}

//加文件夹
function handleAddFolder() {
    const target = fileTree.value[currentFile.value.id];
    if (target.isDir) {
    openFolder(target, fileTree.value, true).then(
        () => {
            store.addFileFolderPending({
                id: target.id,
                type: 'folder',
            })
        },
    );
    } else {
    if (target.parentId !== undefined && !isNaN(target.parentId)) {
        store.addFileFolderPending({
            id: target.parentId,
            type: 'folder',
        })
    } else {
        throw new Error();
    }
    }
}

//加文件
function handleAddFile() {
    let id = currentFile.value.id
    const target = fileTree.value[id];
    if (target.isDir) {
    openFolder(target, fileTree.value, true).then(
        () => {
            store.addFileFolderPending({ id: target.id, type: '' })
        },
    );
    } else {
        if (target.parentId !== undefined && !isNaN(target.parentId)) {
            store.addFileFolderPending({ id: target.parentId, type: '' })
        } else {
            throw new Error();
        }
    }
}

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