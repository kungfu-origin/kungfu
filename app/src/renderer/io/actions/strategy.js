import { deleteStrategy } from '@/io/strategy';
import { deleteTask } from '@/io/base';
import { removeFileFolder } from '__gUtils/fileUtils';
import { STRATEGY_DIR, LOG_DIR } from '__gConfig/pathConfig';

const path = require('path');

//删除策略
export const deleteStrat = (strategyId) => {
    return deleteStrategy(strategyId) //删除策略数据库
    .then(() => deleteTask(strategyId))
    .then(() => removeFileFolder(path.join(STRATEGY_DIR, strategyId))) //策略相关数据
    .then(() => removeFileFolder(path.join(LOG_DIR, strategyId + '.log')))//策略log
}
