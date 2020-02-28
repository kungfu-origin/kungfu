import * as ACCOUNT_API from '__io/kungfu/account';
import { TD_DIR, MD_DIR, buildGatewayPath } from '__gConfig/pathConfig';
import { removeFileFolder } from '__gUtils/fileUtils';
import { startTd, startMd, deleteProcess } from '__gUtils/processUtils';

const path = require('path')

//删除账户需要将所关联的数据库以及进程都关掉
//判断task表和进程中是否存在，有则删除
export const deleteTd = (row: Account): Promise<any> => {
    const { account_id } = row;
    const tdProcessId: string = `td_${account_id}`;
    
    return removeFileFolder(path.join(TD_DIR, account_id.toAccountId()))
    .then(() => removeFileFolder(buildGatewayPath(tdProcessId)))
    .then(() => deleteProcess(tdProcessId))
    .then(() => ACCOUNT_API.deleteTd(account_id)) 
}

export const deleteMd = (row: Account): Promise<any> => {
    const { source_name } = row;
    const mdProcessId: string = `md_${source_name}`;

    return removeFileFolder(path.join(MD_DIR, source_name))
    .then(() => removeFileFolder(buildGatewayPath(mdProcessId)))
    .then(() => deleteProcess(mdProcessId))
    .then(() => ACCOUNT_API.deleteMd(source_name))
}

//起停td
export const switchTd = (account: Account, value: boolean): Promise<any> => {
    const { account_id } = account
    const tdProcessId: string = `td_${account_id}`
    if(!value){
        return deleteProcess(tdProcessId)
        .then((): MessageData => ({ type: 'success', message: '操作成功！' }))       
        .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))
    }

    //改变数据库表内容，添加或修改
    return startTd(account_id)
    .then((): MessageData => ({ type: 'start', message: '正在启动...' }))       
    .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))
}

//起停md
export const switchMd = (account: Account, value: boolean) => {
    const { source_name } = account;
    const mdProcessId: string = `md_${source_name}`
    if(!value){
        return deleteProcess(mdProcessId)
        .then((): MessageData => ({ type: 'success', message: '操作成功！' }))       
        .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))
    }

    //改变数据库表内容，添加或修改
    return startMd(source_name)
    .then((): MessageData => ({ type: 'start', message: '正在启动...' }))       
    .catch((err: Error): MessageData => ({ type: 'error', message: err.message || '操作失败！' }))     
}