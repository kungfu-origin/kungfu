import { defineStore } from 'pinia';
import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  getKfGlobalSettingsValue,
  setKfGlobalSettingsValue,
} from '@kungfu-trader/kungfu-js-api/config/globalSettings';
// import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';

interface ICodeState {
  currentStrategy: string;
  strategyList: KungfuApi.KfConfig[];
  currentFile: Code.FileData;
  entryFile: Code.FileData;
  fileTree: Code.IFileTree;
  globallSetting: Record<string, Record<string, KungfuApi.KfConfigValue>>;
}

export const useCodeStore = defineStore('code', {
  state: (): ICodeState => {
    return {
      currentStrategy: '', //当前运行策略
      strategyList: [], //策略列表
      currentFile: {} as Code.FileData, //文件树高亮
      entryFile: {} as Code.FileData, //入口文件
      fileTree: {}, //文件树
      globallSetting: {}, // kf 配置
    };
  },
  actions: {
    clearAllState() {
      this.currentFile = {};
      this.entryFile = {};
      this.fileTree = {};
    },
    //设置当前策略
    setCurrentStrategy(strategy: Code.Strategy): void {
      this.currentStrategy = strategy;
    },

    setStrategyList(data): void {
      this.strategyList = data;
    },

    //策略编辑，设置当前文件
    setCurrentFile(file: Code.FileData): void {
      this.currentFile = file;
    },

    //策略编辑，设置文件树
    setFileTree(fileTree: Code.IFileTree): void {
      this.fileTree = fileTree;
    },

    //策略编辑，设置文件节点
    setFileNode({
      id,
      attr,
      val,
    }: {
      id: number;
      attr: string;
      val: Code.FileIds | boolean;
    }): void {
      const fileTree = this.fileTree;
      const node = deepClone(fileTree[id]);
      node[attr] = val;
      this.fileTree[id] = node;
    },

    //策略编辑，添加文件或文件夹时，添加“pending”
    addFileFolderPending({ id, type }): void {
      const targetChildren = this.fileTree[id].children;
      if (type == 'folder') {
        targetChildren['folder'].unshift('pending');
      } else {
        targetChildren['file'].unshift('pending');
      }
      this.fileTree[id]['children'] = targetChildren;
      this.fileTree['pending']['parentId'] = id;
      const currentId: number =
        this.fileTree[id].parentId == 0 ? id : this.fileTree[id].parentId;
      if (this.fileTree[currentId].fileId) {
        this.fileTree[currentId].fileId++;
      } else {
        this.fileTree[currentId].fileId = 1;
      }
    },

    //策略编辑时，添加文件或文件夹时，删除“pending”
    removeFileFolderPending({ id, type }): void {
      const targetChildren = this.fileTree[id].children;
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

      this.fileTree[id]['children'] = targetChildren;
      this.fileTree['pending']['parentId'] = '';
    },

    //标记入口文件
    setEntryFile(entryFile: Code.FileData): void {
      this.entryFile = entryFile;
    },

    async getKungfuConfig(): Promise<void> {
      const globallSetting = getKfGlobalSettingsValue();
      await this.setKungfuConfig(globallSetting);
    },

    async setGlobalSetting(
      globallSetting: Record<string, Record<string, KungfuApi.KfConfigValue>>,
    ): Promise<void> {
      await this.setKungfuConfig(globallSetting);
      setKfGlobalSettingsValue(this.globallSetting);
      await this.getKungfuConfig();
    },

    setKungfuConfig(
      globallSetting: Record<string, Record<string, KungfuApi.KfConfigValue>>,
    ): void {
      Object.keys(globallSetting || {}).forEach((key) => {
        this.globallSetting[key] = globallSetting[key];
      });
    },
  },
  getters: {
    getRootFileId(): string {
      for (const id in this.fileTree) {
        if (this.fileTree[id].root) {
          return id;
        }
      }
      return '';
    },
  },
});
