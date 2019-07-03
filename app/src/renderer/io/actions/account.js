import * as ACCOUNT_API from '@/io/db/account';
import { TD_DIR, buildGatewayPath } from '__gConfig/pathConfig';
import { removeFileFolder } from '__gUtils/fileUtils';
import { startTd, startMd, deleteProcess } from '__gUtils/processUtils';
import { setTasksDB, deleteTask } from '@/io/db/base';

const path = require('path')

//删除账户需要将所关联的数据库以及进程都关掉
//判断task表和进程中是否存在，有则删除
//TODO
export const deleteAccount = (row, accountList = []) => {
    const { account_id, source_name, receive_md } = row
    //查看该账户下是否存在task中的td任务
    const tdProcessId = `td_${account_id}`
    const mdProcessId = `md_${source_name}`
    const leftAccounts = accountList.filter(a => a.source_name === source_name && a.receive_md === 0 && a.account_id !== account_id)

    //删除td
    return removeFileFolder(path.join(TD_DIR, account_id.toAccountId()))
    .then(() => removeFileFolder(buildGatewayPath(tdProcessId)))
    .then(() => deleteProcess('td_' + row.account_id))
    .then(() => deleteTask(tdProcessId))
    .then(() => {if(receive_md) return removeFileFolder(buildGatewayPath(mdProcessId))})
    .then(() => {if(receive_md) return deleteProcess('md_' + row.source_name)})                     
    .then(() => {if(receive_md) return deleteTask(mdProcessId)})
    .then(() => ACCOUNT_API.deleteAccount(account_id))//删除账户表中的数据    
    .then(() => {
        if(receive_md && leftAccounts.length) {
            return ACCOUNT_API.changeAccountMd(leftAccounts[0].account_id, true)
        }
        return false;
    })
}

//起停td
export const switchTd = (account, value) => {
    const {account_id, config} = account
    const tdProcessId = `td_${account_id}`
    if(!value){
        return deleteProcess(tdProcessId)
        .then(() => deleteTask(tdProcessId))
        .then(() => ({ type: 'success', message: '操作成功！' }))       
        .catch(err => ({ type: 'error', message: err || '操作失败！' }))
    }

    //改变数据库表内容，添加或修改
    return setTasksDB({name: tdProcessId, type: 'td', config})   
    .then(() => startTd(account_id)) //开启td,pm2
    .then(() => ({ type: 'start', message: '正在启动...' }))       
    .catch(err => ({ type: 'error', message: err.message || '操作失败！' }))
}

//起停md
export const switchMd = (account, value) => {
    const {source_name, config} = account;
    const mdProcessId = `md_${source_name}`
    if(!value){
        return deleteProcess(mdProcessId)
        .then(() => deleteTask(mdProcessId))
        .then(() => ({ type: 'success', message: '操作成功！' }))       
        .catch(err => ({ type: 'error', message: err || '操作失败！' }))
    }

    //改变数据库表内容，添加或修改
    return setTasksDB({name: mdProcessId, type: 'md', config}) 
    .then(() => startMd(source_name)) //开启td,pm2
    .then(() => ({ type: 'start', message: '正在启动...' }))       
    .catch(err => ({ type: 'error', message: err.message || '操作失败！' }))     
}