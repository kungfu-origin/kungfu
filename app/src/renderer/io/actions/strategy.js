import { deleteStrategy, getStrategyById } from '@/io/db/strategy';
import { removeFileFolder } from '__gUtils/fileUtils';
import { deleteProcess, startStrategy } from '__gUtils/processUtils';
import { STRATEGY_DIR, LOG_DIR } from '__gConfig/pathConfig';
import { deleteTask } from '@/io/db/base';

const path = require('path');

//删除策略
export const deleteStrat = (strategyId) => {
    return deleteStrategy(strategyId) //删除策略数据库
    .then(() => deleteTask(strategyId))
    .then(() => removeFileFolder(path.join(STRATEGY_DIR, strategyId))) //策略相关数据
    .then(() => removeFileFolder(path.join(LOG_DIR, strategyId + '.log')))//策略log
}


export const switchStrategy = (strategyId, value) => {
    if(!value){
        return deleteProcess(strategyId)
        .then(() => ({ type: 'success', message: '操作成功！' }))       
        .catch(err => ({ type: 'error', message: err || '操作失败！' }))
    }

    // for import file changed in code editor module
    return getStrategyById(strategyId).then(curStrategy => {
        if(!curStrategy.length) return new Promise(resolve => resolve({ type: 'error', message: `${strategyId} is not existed!` })); 
        const strategyPath = curStrategy[0].strategy_path;
        if(!strategyPath) return new Promise(resolve => resolve({ type: 'error', message: `该策略未绑定任何文件！` }))
        const config = JSON.stringify({ strategy_id: strategyId, strategy_path: strategyPath })
        return startStrategy(strategyId, strategyPath)// 启动策略
        .then(() => ({ type: 'start', message: '正在启动...' }))       
        .catch(err => ({ type: 'error', message: err || '操作失败！' }))
    })
}