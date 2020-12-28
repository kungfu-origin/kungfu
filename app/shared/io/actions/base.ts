import { startCustomProcess, deleteProcess, killKfc, startMaster, startLedger, startProcessLoopGetStatus } from '__gUtils/processUtils';
import { delayMiliSeconds } from '__gUtils/busiUtils';
import { buildCustomProcessConfig } from '__gConfig/systemConfig';
import { KF_TARADING_CONFIG_PATH, KF_CONFIG_PATH } from '__gConfig/pathConfig';
import { readJsonSync } from "__gUtils/fileUtils";

export const switchMaster = async (status: boolean): Promise<any> => {
    if(!status){
        try {
            await deleteProcess('master');
            await killKfc()
        } catch(err) {
            throw err
        }
        return
    } 
    try {

        const kfSystemConfig: any = readJsonSync(KF_CONFIG_PATH) || {};
        const ifCleanBeforeLaunchMaster = ((kfSystemConfig.performance || {}).cleanBeforeLaunchMaster) || false;
        const ifArchive = ((kfSystemConfig.performance || {}).archive) || false;
        const archiveParams = ifArchive ? '-A' : ''

        if (ifCleanBeforeLaunchMaster) {
            await startProcessLoopGetStatus({
                "name": 'journalClean',
                "args": `journal clean ${archiveParams}`
            }, () => {})
        }

        await startMaster(false)
        await delayMiliSeconds(1000)
        await startLedger(false)
    } catch (err) {
        throw err
    }
}

export const switchLedger = (status: boolean): Promise<any> => {
    if(!status) return deleteProcess('ledger')   
    return startLedger(false)
}

export const switchCustomProcess = (status: boolean, targetName: string) => {
    if(!status) return deleteProcess(targetName)

    const customProcessConfig = buildCustomProcessConfig();
    const targetProcessConfig = customProcessConfig[targetName];
    if(!targetProcessConfig) throw new Error(`No ${targetName} in systemConfig systemTradingConfig or extensionConfig`)
    const kfSystemConfig = readJsonSync(KF_CONFIG_PATH) || {};
    const kfSystemTradingConfig = readJsonSync(KF_TARADING_CONFIG_PATH) || {};
    const systemConfigValData: any = {
        ...kfSystemConfig,
        ...kfSystemTradingConfig
    }
    const args = targetProcessConfig.args;
    const parentKey = targetProcessConfig.parentKey;
    const processValData = systemConfigValData[parentKey];

    const params = args
        .map((arg: SystemConfigChildArgsItemData): string => {
            const key = arg.key;
            const valueKey = arg.value;
            return `${key} ${processValData[valueKey]}`;
        });
        
    return startCustomProcess(targetName, params.join(' '))
}
