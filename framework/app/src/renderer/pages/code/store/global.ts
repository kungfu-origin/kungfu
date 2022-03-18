import { defineStore } from 'pinia';
import fse from 'fs-extra';

import { KF_CONFIG_PATH } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';

interface ICodeState {
  currentStrategy: string;
  strategyList: KungfuApi.KfConfig[];
  currentFile: FileProps;
  //   entryFile: any;
  fileTree: Record<string, Record<string, FileProps>>;
  kfConfig: Record<string, Record<string, KungfuApi.KfConfigValue>>;
}

export const useCodeStore = defineStore('code', {
  state: (): ICodeState => {
    return {
      currentStrategy: '', //当前运行策略
      strategyList: [], //策略列表
      currentFile: {}, //文件树高亮
      //   entryFile: {}, //入口文件
      fileTree: {}, //文件树
      kfConfig: {}, // kf 配置
    };
  },
  actions: {
    //设置当前策略
    setCurrentStrategy(strategy) {
      this.state.currentStrategy = strategy;
    },

    setStrategyList() {
      getAllKfConfigOriginData().then((res) => {
        const { strategy } = res;
        this.state.strategyList = strategy;
      });
    },

    //策略编辑，设置当前文件
    setCurrentFile(file) {
      this.state.currentFile = file;
    },

    //策略编辑，设置文件树
    setFileTree(fileTree) {
      this.state.fileTree = fileTree;
    },

    //策略编辑，设置文件节点
    setFileNode({ id, attr, val }) {
      const fileTree = this.state.fileTree;
      const node = deepClone(fileTree[id]);
      node[attr] = val;
      this.state.fileTree[id] = node;
    },

    //策略编辑，添加文件或文件夹时，添加“pending”
    addFileFolderPending({ id, type }) {
      const targetChildren = this.state.fileTree[id].children;
      if (type == 'folder') {
        targetChildren['folder'].unshift('pending');
      } else {
        targetChildren['file'].unshift('pending');
      }
      this.state.fileTree[id]['children'] = targetChildren;
      this.state.fileTree['pending']['parentId'] = id;
    },

    //策略编辑时，添加文件或文件夹时，删除“pending”
    removeFileFolderPending({ id, type }) {
      const targetChildren = this.state.fileTree[id].children;
      if (type == 'folder') {
        targetChildren['folder'].splice(
          targetChildren['folder'].indexOf('pending'),
          1,
        );
      } else {
        targetChildren['file'].splice(
          targetChildren['file'].indexOf('pending'),
          1,
        );
      }
      this.state.fileTree[id]['children'] = targetChildren;
      this.state.fileTree['pending']['parentId'] = '';
    },

    //标记入口文件
    setEntryFile(entryFile) {
      this.state.entryFile = entryFile;
    },

    async getKungfuConfig() {
      const kfConfig = fse.readJsonSync(KF_CONFIG_PATH);
      await this.setKungfuConfig(kfConfig);
    },

    setKungfuConfig(kfConfig) {
      Object.keys(kfConfig || {}).forEach((key) => {
        this.state.kfConfig[key] = kfConfig[key];
      });
    },

    setKungfuConfigByKeys(kfConfig) {
      Object.keys(kfConfig || {}).forEach((key) => {
        this.state.kfConfig[key] = kfConfig[key];
      });
      fse.outputJsonSync(KF_CONFIG_PATH, {
        ...this.state.kfConfig,
        ...kfConfig,
      });
    },
  },
});
