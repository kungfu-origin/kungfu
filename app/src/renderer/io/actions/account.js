import * as ACCOUNT_API from '@/io/account';
import { deleteTask } from '@/io/base';
import { ACCOUNTS_DIR, buildGatewayPath } from '__gConfig/pathConfig';
import { removeFileFolder } from '__gUtils/fileUtils';
import { deleteProcess } from '__gUtils/processUtils';

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
    return removeFileFolder(path.join(ACCOUNTS_DIR, account_id.toAccountId()))
    .then(() => removeFileFolder(buildGatewayPath(tdProcessId)))
    .then(() => deleteProcess('td_' + row.account_id))
    .then(() => deleteTask(tdProcessId))
    .then(() => { if(receive_md) return removeFileFolder(buildGatewayPath(mdProcessId)) })
    .then(() => { if(receive_md) return deleteProcess('md_' + row.source_name) })                     
    .then(() => { if(receive_md) return deleteTask(mdProcessId) })
    .then(() => ACCOUNT_API.deleteAccount(account_id))//删除账户表中的数据    
    .then(() => {
        if(receive_md && leftAccounts.length) {
            return ACCOUNT_API.changeAccountMd(leftAccounts[0].account_id, true)
        }
        return false;
    })
}