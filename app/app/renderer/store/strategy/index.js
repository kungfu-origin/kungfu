import * as actions from './actions';
import * as mutations from './mutations';
import * as getters from './getters';

export default {
    state: {
        currentStrategy: '', //当前运行策略
        strategyList: [], //策略列表
        currentFile: {}, //文件树高亮
        entryFile: {}, //入口文件
        fileTree: {}, //文件树
    },
    actions,
    mutations,
    getters
}
