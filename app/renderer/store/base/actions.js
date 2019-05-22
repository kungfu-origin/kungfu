import Vue from 'vue';
import * as BASE_API from '@/io/base';
import { nanoGetCalendar } from '@/io/nano/nanoReq';

export const setCalendar = ({commit}, calendar) => {
    commit('SET_CALENDAR', calendar)
}

export const setProcessStatus = ({commit}, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

//获取tasks
export const getTasks = ({commit}) => {
    return new Promise(resolve => {
        BASE_API.getTaskList().then(res => {
            let tasks = {};
            res.map(r => tasks[r.name] = r)
            commit("SET_TASKS", tasks)
            resolve(res)
        }).catch(err => {
            Vue.$message.error(err.message || '操作失败！')
        })
    })
}

//判断是添加还是修改表内容
//检查数据库中是否有该条数据，有则只修改config,没有则添加一条数据
export const setTasksDB = ({state}, {name, type, config}) => {
    if(!state.tasks[name]) {
        return BASE_API.addTask(name, type, config)
    }else {
        return BASE_API.updateTask(name, config)
    }
}

//删除任务
export const deleteTask = ({dispatch, state}, taskName) => {
    return new Promise (resolve => {
        if(!state.tasks[taskName]) {
            resolve()
            return
        }  
        BASE_API.deleteTask(taskName).then(() => {
            dispatch('getTasks')
            resolve()
        })
    })
}

//主动获得交易日
export const getCalendar = ({dispatch}) => {
    return nanoGetCalendar().then((calendar) => {
        dispatch('setCalendar', calendar)
        return calendar
    })
}




