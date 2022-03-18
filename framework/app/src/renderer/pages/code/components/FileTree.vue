<template>
  <div class="file-tree">
    <div class="open-editor-folder">
      设置策略入口文件
    </div>
    <div class="file-tree-content">
      <div class="strategy-name">
        <span class="name">
          <span>sssss</span>
          （当前策略）
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
      <!-- <ul class="file-tree-body" v-if="strategyPath">
        <fileNode
          v-for="item in fileTree"
          :fileNode="item"
          :key="item.fileId"
          v-if="item.root"
          :id="item.id"
          type="folder"
        ></fileNode>
      </ul> -->
    </div>
  </div>
</template>
<script setup lang="ts">
// import { mapState } from 'vuex';
import path from 'path';

import FileNode from './FileNode.vue';

// import * as CODE_UTILS from '__gUtils/fileUtils';
// import { ipcEmitDataByName } from '@/ipcMsg/emitter';

// export default {
//   props: {
//     strategy: {
//       type: Object,
//       default: {},
//     },
//   },

//   components: {
//     FileNode,
//   },

//   data() {
//     return {
//       strategyPath: '',
//       strategyPathName: '',
//     };
//   },

//   computed: {
//     ...mapState({
//       fileTree: (state) => state.fileTree,
//       currentFile: (state) => state.currentFile,
//     }),
//   },

//   watch: {
//     strategy(newStrategy) {
//       this.getPath(newStrategy);
//       this.initFileTree(newStrategy).then((fileTree) => {
//         //设置current file
//         const entryPath = newStrategy.strategy_path; //入口文件
//         const currentFile = Object.values(fileTree || {}).filter(
//           (f) => f.filePath == entryPath,
//         )[0];
//         if (currentFile) {
//           this.$store.dispatch('setEntryFile', currentFile);
//           this.$store.dispatch('setCurrentFile', currentFile);
//         }
//       });
//     },
//   },

//   methods: {
//     //绑定策略
//     handleBindStrategyFolder() {
//       const dialog = require('electron').remote.dialog;
//       dialog.showOpenDialog(
//         {
//           properties: ['openFile'],
//         },
//         (strategyPath) => {
//           if (!strategyPath || !strategyPath[0]) return;
//           if (!this.strategy.strategy_id) return;
//           this.bindStrategyPath(strategyPath);
//         },
//       );
//     },

//     //加文件夹
//     handleAddFolder() {
//       const t = this;
//       const target = this.fileTree[this.currentFile.id];
//       if (target.isDir) {
//         CODE_UTILS.openFolder(this.$store, target, this.fileTree, true).then(
//           () => {
//             this.$store.dispatch('addFileFolderPending', {
//               id: target.id,
//               type: 'folder',
//             });
//           },
//         );
//       } else {
//         if (target.parentId !== undefined && !isNaN(target.parentId)) {
//           this.$store.dispatch('addFileFolderPending', {
//             id: target.parentId,
//             type: 'folder',
//           });
//         } else {
//           throw new Error();
//         }
//       }
//     },

//     //加文件
//     handleAddFile() {
//       const t = this;
//       const target = this.fileTree[this.currentFile.id];
//       if (target.isDir) {
//         CODE_UTILS.openFolder(this.$store, target, this.fileTree, true).then(
//           () => {
//             this.$store.dispatch('addFileFolderPending', { id: target.id });
//           },
//         );
//       } else {
//         if (target.parentId !== undefined && !isNaN(target.parentId)) {
//           this.$store.dispatch('addFileFolderPending', { id: target.parentId });
//         } else {
//           throw new Error();
//         }
//       }
//     },

//     //bind data中path 与 sqlite中path
//     async bindStrategyPath(strategyPath) {
//       await ipcEmitDataByName('updateStrategyPath', {
//         strategyId: this.strategy.strategy_id,
//         strategyPath: strategyPath[0],
//       });

//       this.$message.success(
//         `策略${this.strategy.strategy_id}文件路径修改成功！`,
//       );
//       //每次更新path，需要通知root组件更新stratgy
//       this.$emit('updateStrategy');
//     },

//     //通过新打开的窗口获取文件树
//     async initFileTree(strategy) {
//       const strategyId = strategy.strategy_id;
//       const strategyPath = strategy.strategy_path;
//       if (!strategyId || !strategyPath) return;
//       //根文件夹得信息不像其他通过fs读取，而是直接从props的strategy中去取
//       const rootId = window.fileId++;
//       const rootFile = CODE_UTILS.buildFileObj({
//         id: rootId,
//         isDir: true,
//         name: this.strategyPathName,
//         filePath: this.strategyPath,
//         root: true,
//         open: true,
//       });
//       //获取第一级文件树
//       let ids, fileTree;
//       try {
//         const fileTreeData = await CODE_UTILS.getTreeByFilePath(
//           rootFile,
//           fileTree,
//         );
//         ids = fileTreeData.ids;
//         fileTree = fileTreeData.fileTree;
//       } catch (err) {
//         this.$message.error(err);
//       }
//       //处理根
//       rootFile['children'] = ids;
//       fileTree[rootId] = rootFile;
//       //pending
//       fileTree = this.bindFunctionalNode(fileTree);
//       this.$store.dispatch('setFileTree', fileTree);
//       this.$store.dispatch('setCurrentFile', rootFile);
//       return fileTree;
//     },

//     //增加 以pending 为key 的node，为addfile做准备
//     bindFunctionalNode(fileTree) {
//       fileTree['pending'] = {
//         parentId: '',
//       };
//       return fileTree;
//     },

//     //从prop内获取path
//     getPath(strategy) {
//       if (strategy && strategy.strategy_path) {
//         //因为绑定策略时是文件，需要提取其父目录
//         this.strategyPath = path.dirname(strategy.strategy_path);
//         this.strategyPathName = path.basename(this.strategyPath);
//       }
//     },
//   },
// };
</script>

<style lang="less">
.file-tree {
    width: 100%;
}
</style>