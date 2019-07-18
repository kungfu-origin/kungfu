import { runSelectDB, runInsertUpdateDeleteDB } from '__gUtils/dbUtils';
import { TASKS_DB } from '__gConfig/pathConfig';

interface Task {
    name: string;
    type: string;
    config: any;
}

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

export const addTask = (name: string, type: string, config: any) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'INSERT INTO kungfu_task(name, task_type, config) VALUES (?, ?, ?)', [name, type, config])
}

/**
 * 修改task
 * @param {String} name 
 */

export const updateTask = (name: string, config: any) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'UPDATE kungfu_task SET config = ? WHERE name = ?', [config, name])
}

/**
 * 删除task
 * @param {String} name 
 */
export const deleteTask = (name: string) => {
    return runInsertUpdateDeleteDB(TASKS_DB, 'DELETE FROM kungfu_task WHERE name = ?', name)
}

//判断是添加还是修改表内容
//检查数据库中是否有该条数据，有则只修改config,没有则添加一条数据
export const setTasksDB = ({ name, type, config }: Task) => {
    return getTaskList().then((list: Task[]) => {
        const existed = list.filter((l: Task): boolean => l.name === name).length;
        if(!existed) return addTask(name, type, config)
        else return updateTask(name, config)
    })
}
