
import { KUNGFU_ENGINE_PATH } from '__gConfig/pathConfig';
import { platform } from '__gConfig/platformConfig';

const path = require('path');
const colors = require('colors');
const os = require('os');
const { execFile } = require('child_process');
const inquirer = require( 'inquirer' );
const { PathPrompt } = require('@/assets/static/js/inquirer-path/lib');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));
inquirer.registerPrompt('path', PathPrompt);

const inquirerGetPath = async () => {
    const { extPath } = await inquirer.prompt([
        {
            type: 'path',
            name: 'extPath',
            default: os.homedir(),
            message: 'Enter extension path (.tgz file)'
        }
    ])
    return extPath
} 

const inquirerRemoveByName = async (extList: string[]) => {
    const { extName } = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'extName',
            message: 'Select extension to remove',
            source: async () => extList
        }
    ])
    return (extName.split(' ')[0] || "");
}

const execKfcOfArgs = (args: string[]): Promise<string> => {
    console.log(path.join(KUNGFU_ENGINE_PATH, 'kfc'))
    return new Promise((resolve, reject) => {
        const kfcName = platform === 'win' ? 'kfc.exe' : 'kfc'
        execFile(
            path.join(KUNGFU_ENGINE_PATH, 'kfc', kfcName),
            args,
            { shell: true },
            (error: Error, stdout: string, stderr: string) => {
                if(error) {
                    reject(new Error([error.message, stdout].join('\n')))
                    return;                    
                }
                resolve(stdout)
            }
        )
    })
}

const execAddExtension = (extPath: string): Promise<string> => {
    return execKfcOfArgs([` ext install -f ${extPath}`])
}

const execListExtension = (): Promise<string> => {
    return execKfcOfArgs([` ext list`])
}

const execRemoveExtension = (targetName: string): Promise<string> => {
    return execKfcOfArgs([` ext uninstall -n ${targetName}`])

}

export const addExtension = async () => {
    try {
        const extPath = await inquirerGetPath();
        const stdout = await execAddExtension(extPath)
        console.success(stdout)
    } catch (err) {
        throw err
    }
} 

export const listExtension = async (ifStdout = true): Promise<void | string> => {
    try {
        const stdout: string = await execListExtension();
        if(ifStdout) console.log(stdout) 
        else return stdout
    } catch (err) {
        throw err
    }
}

export const removeExtension = async () => {
    try {
        const list: any = await listExtension(false);
        const extList = list.split('\n').filter((l: string) => !!l).slice(1);
        const targetName = await inquirerRemoveByName(extList)
        const stdout = await execRemoveExtension(targetName)
        console.success(stdout)
    } catch (err) {
        throw err
    }
}