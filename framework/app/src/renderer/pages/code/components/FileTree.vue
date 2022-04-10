<template>
  <div class="file-tree">
    <a-button
      type="default"
      class="open-editor-folder"
      @click="handleBindStrategyFolder"
    >
      设置策略入口文件
    </a-button>
    <div class="file-tree-content">
      <div class="strategy-name">
        <span class="name">
          <span v-if="strategy">{{ strategy.strategy_id }}</span>
          （当前策略)
        </span>
        <span class="tree-deal-file">
          <span
            class="create"
            title="新建文件"
            v-if="strategyPath"
            @click="handleAddFile"
          >
            <FileAddFilled class="icon" />
          </span>
          <span
            class="create"
            title="新建文件夹"
            v-if="strategyPath"
            @click="handleAddFolder"
          >
            <FolderAddFilled class="icon" />
          </span>
        </span>
      </div>
      <div class="file-tree-body" v-if="strategyPath">
        <div class="scroll-view">
          <div v-for="file in fileTree">
            <FileNode
              v-if="file.root"
              :count="0"
              :fileNode="file"
              :id="file.id"
              type="folder"
            ></FileNode>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
export default {
  emits: ['updateStrategy'],
};
</script>
<script setup lang="ts">
import {
  watch,
  ref,
  getCurrentInstance,
  ComponentInternalInstance,
  toRefs,
} from 'vue';
import path from 'path';
import { storeToRefs } from 'pinia';
import { dialog } from '@electron/remote';
import { message } from 'ant-design-vue';
import { getTreeByFilePath } from '../../../assets/methods/codeUtils';
import { useCodeStore } from '../store/codeStore';
import FileNode from './FileNode.vue';
import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { openFolder, buildFileObj } from '../../../assets/methods/codeUtils';

import { FileAddFilled, FolderAddFilled } from '@ant-design/icons-vue';
import { ipcEmitDataByName } from '../../../ipcMsg/emitter';

const store = useCodeStore();
const props = defineProps<{
  strategy: Code.Strategy;
}>();
const { strategy } = toRefs(props);
const strategyPath = ref<string>('');
const strategyPathName = ref<string>('');
const { proxy } = getCurrentInstance() as ComponentInternalInstance;
const { currentFile, fileTree } = storeToRefs(useCodeStore());

watch(strategy.value as Code.Strategy, (newStrategy) => {
    console.log(newStrategy);
    
  getPath(newStrategy);
  initFileTree(newStrategy).then((fileItem) => {
    const entryPath: string = newStrategy.strategy_path;

    const currentFile = findTargetFromArray<Code.FileData>(
      Object.values(fileItem),
      'filePath',
      entryPath,
    );

    if (currentFile) {
      store.setEntryFile(currentFile);
      store.setCurrentFile(currentFile);
    }
  });
});

//绑定策略
function handleBindStrategyFolder() {
  dialog
    .showOpenDialog({
      properties: ['openFile'],
    })
    .then((strategyPath) => {
      if (!strategyPath || !strategyPath.filePaths[0]) return;
      if (!strategy.value?.strategy_id) return;
      bindStrategyPath(strategyPath.filePaths[0]);
    });
}

//bind data中path 与 sqlite中path
async function bindStrategyPath(strategyPathNew) {
    if (strategy && strategy.value.strategy_id) {
        await ipcEmitDataByName('updateStrategyPath', {
            strategyId: strategy.value.strategy_id,
            strategyPath: strategyPathNew,
        })
        message.success(
            `策略${strategy.value.strategy_id}文件路径修改成功！`,
        );
        //每次更新path，需要通知root组件更新stratgy
        updateStrategyToApp(strategyPathNew)
    }
}

function updateStrategyToApp(strategyPath) {
    proxy?.$emit('updateStrategy', strategyPath);

}

//加文件夹
function handleAddFolder() {
  const id: number = currentFile.value.id;
  const rootId: number = +store.getRootFileId;
  const target = fileTree.value[id] || fileTree.value[rootId];
  if (target.isDir) {
    openFolder(target, fileTree.value, true).then(() => {
      store.addFileFolderPending({ id: target.id, type: 'folder' });
    });
  } else {
    if (target.parentId !== undefined && !isNaN(target.parentId)) {
      store.addFileFolderPending({
        id: target.parentId,
        type: 'folder',
      });
    } else {
      throw new Error();
    }
  }
}

//加文件
function handleAddFile() {
  const id = currentFile.value.id;
  const rootId: number = +store.getRootFileId;
  const target = fileTree.value[id] || fileTree.value[rootId];
  if (target.isDir) {
    openFolder(target, fileTree.value, true).then(() => {
      store.addFileFolderPending({ id: target.id, type: '' });
    });
  } else {
    if (target.parentId !== undefined && !isNaN(target.parentId)) {
      store.addFileFolderPending({ id: target.parentId, type: '' });
    } else {
      throw new Error();
    }
  }
}

//从prop内获取path
function getPath(strategy: Code.Strategy) {
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
    children: { file: [], folder: [] },
    stats: {},
    root: true,
    open: true,
    fileId: 1,
  });
  //获取第一级文件树
  let ids, rootFileTree;
  try {
    const fileTreeData = await getTreeByFilePath(rootFile, rootFileTree);
    ids = fileTreeData.ids;
    rootFileTree = fileTreeData.fileTree;
  } catch (err) {
    message.error(err);
  }

  // 处理根
  rootFile['children'] = ids;
  rootFileTree[rootId] = rootFile;
  // padding
  rootFileTree = bindFunctionalNode(rootFileTree);
  store.setFileTree(rootFileTree);
  store.setCurrentFile(rootFile);

  return rootFileTree;
}

//增加 以pending 为key 的node，为addfile做准备
function bindFunctionalNode(curFileTree) {
  curFileTree['pending'] = {
    parentId: '',
  };
  return curFileTree;
}
</script>

<style lang="less">
.file-tree {
  width: 300px;
  padding-top: 8px;
  text-align: center;
  display: flex;
  flex-direction: column;
  background: @component-background;

  .open-editor-folder {
    width: 90%;
    height: 36px;
    margin: auto;
  }

  .strategy-name {
    font-size: 14px;
    font-weight: bolder;
    margin-top: 8px;
    height: 30px;
    line-height: 30px;
    padding: 0px 8px;
    border-top: none;
    border-bottom: 2px solid @component-background;
    text-align: left;
    box-sizing: border-box;
    color: @text-color;
    .name {
      span {
        vertical-align: bottom;
        display: inline-block;
        max-width: 160px;
        text-overflow: ellipsis;
        white-space: nowrap;
        overflow: hidden;
      }
    }
    .tree-deal-file {
      display: none;
    }
    .folder-oper {
      cursor: pointer;
      display: none;
      padding: 0 6px;
      i {
        font-size: 15px;
      }
    }
  }
  .folder-name {
    height: 30px;
    line-height: 30px;
    color: @text-color;
    text-align: left;
    display: flex;
    flex-direction: row;
    padding-left: 5px;
    .name {
      max-width: 100px;
      font-size: 14px;
      padding-right: 20px;
      color: @text-color;
    }
    .title {
      color: @input-bg;
    }
  }
  .file-tree-content {
    height: calc(100% - 40px);
  }
  .file-tree-body {
    height: calc(100% - 38px);
    overflow: auto;
    padding-left: 5px;
    .scroll-view {
      overflow-y: scroll;
    }
  }
  &:hover {
    .tree-deal-file {
      display: block;
      float: right;
      margin-right: 10px;
      .create {
        margin: 0 2px;
        cursor: pointer;
        .icon {
          color: @text-color;
          &:hover {
            color: @icon-color-hover;
          }
        }
      }
    }
  }
}
</style>
