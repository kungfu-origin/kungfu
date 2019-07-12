import { deleteStrategy, getStrategyById } from '__io/db/strategy';
import { removeFileFolder } from '__gUtils/fileUtils';
import { deleteProcess, startStrategy } from '__gUtils/processUtils';
import { STRATEGY_DIR, LOG_DIR } from '__gConfig/pathConfig';
import { deleteTask } from '__io/db/base';

const path = require('path');

//删除策略
export const deleteStrat = (strategyId: string): Promise<void> => {
    return deleteStrategy(strategyId) //删除策略数据库
    .then(() => deleteTask(strategyId))
    .then(() => removeFileFolder(path.join(STRATEGY_DIR, strategyId))) //策略相关数据
    .then(() => removeFileFolder(path.join(LOG_DIR, strategyId + '.log')))//策略log
}


export const switchStrategy = (strategyId: string, value: boolean): Promise<MessageData> => {
    if(!value){
        return deleteProcess(strategyId)
        .then((): MessageData => ({ type: 'success', message: '操作成功！' }))       
        .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))
    }

    // for import file changed in code editor module
    return getStrategyById(strategyId).then((curStrategy: Strategy[]): Promise<MessageData> => {
        const notExistMessage: MessageData = { type: 'error', message: `${strategyId} is not existed!` }
        if(!curStrategy.length) return new Promise(resolve => resolve(notExistMessage)); 
        const strategyPath = curStrategy[0].strategy_path;
        const noStrategyPath: MessageData = { type: 'error', message: `该策略未绑定任何文件！` }
        if(!strategyPath) return new Promise(resolve => resolve(noStrategyPath))
        return startStrategy(strategyId, strategyPath)// 启动策略
        .then((): MessageData => ({ type: 'start', message: '正在启动...' }))       
        .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))
    })
}