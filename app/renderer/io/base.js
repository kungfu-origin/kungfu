import {runSelectDB, runInsertUpdateDeleteDB} from '__gUtils/dbUtils';
import {TASKS_DB} from '__gConfig/pathConfig';
/**
 * 获取md td运行状态列表
 */
export const getTaskList = () => {
    return runSelectDB(TASKS_DB, 'SELECT * FROM kungfu_task')
}


/**
 * 新建task
 * @param {String} name 
 */

export const addTask = (name, type, config) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'INSERT INTO kungfu_task(name, task_type, config) VALUES (?, ?, ?)', [name, type, config])
}

/**
 * 修改task
 * @param {String} name 
 */

export const updateTask = (name, config) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'UPDATE kungfu_task SET config = ? WHERE name = ?', [config, name])
}
/**
 * 删除task
 * @param {String} name 
 */

export const deleteTask = (name) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'DELETE FROM kungfu_task WHERE name = ?', name)
}

