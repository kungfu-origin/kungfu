/*
 * @Author: your name
 * @Date: 2020-06-11 16:09:35
 * @LastEditTime: 2020-06-11 16:27:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/commanders/shutdown.ts
 */ 

import { killExtra, killGodDaemon, killKfc, kfKill } from '__gUtils/processUtils';
import { KF_HOME } from '__gConfig/pathConfig';



const glob = require('glob');
const fs = require('fs');
const path = require('path');

function deleteNNFiles () {
    return new Promise((resolve, reject) => {
        glob('**/*.nn', {
            cwd: KF_HOME
        }, (err: Error, files: any) => {
            if (err) {
                reject(err)
            }
    
            files.forEach((file: any) => {
                console.log('Deleting ', path.join(KF_HOME, file))
                fs.unlinkSync(path.join(KF_HOME, file))
            });

            resolve(true)
        })
    })
    
}

 export const shutdown = async () => {
    try {
        console.log('Killing kfc ...')
        await killKfc();
        console.log('Killing pm2 ...')
        await killGodDaemon();
        await kfKill(['pm2']);
        console.log('Killing extra ...')
        await killExtra();
        await deleteNNFiles();
        
        console.success(`Shutdown kungfu`)
        process.exit(0)
    } catch (err) {
        console.error(err)
        process.exit(1)
    }
 }