/*
 * @Author: your name
 * @Date: 2020-06-16 20:28:28
 * @LastEditTime: 2020-06-16 20:35:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli/src/commanders/monitKill.ts
 */ 
/*
 * @Author: your name
 * @Date: 2020-06-11 16:09:35
 * @LastEditTime: 2020-06-16 20:27:36
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli/src/commanders/shutdown.ts
 */ 

import { kfKill } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { SYSTEM_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { deleteNNFiles } from '@kungfu-trader/kungfu-cli/commanders/shutdown';




const glob = require('glob');
const fs = require('fs');
const path = require('path');



 export const monitKill = async () => {
    try {
        console.log('Killing monit ...')
        await kfKill(['monit']);
        const nodePath = path.join(SYSTEM_DIR, 'node')
        await deleteNNFiles(nodePath);
        
        console.success(`Kill Monitors`)
        process.exit(0)
    } catch (err) {
        console.error(err)
        process.exit(1)
    }
 }