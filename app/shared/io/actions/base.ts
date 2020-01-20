import { startCustomProcess, deleteProcess, killKfc, startMaster, startLedger, startBar } from '__gUtils/processUtils';
import { delayMiliSeconds } from '__gUtils/busiUtils';
import { KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { readJsonSync } from "__gUtils/fileUtils";

export const switchMaster = async (status: boolean): Promise<any> => {
    if(!status){
        try {
            await deleteProcess('master');
            await killKfc()
        } catch(err) {
            throw err
        }
    } 
    try {
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

export const switchCustomProcess = (status: boolean, targetName: string, params = ''): Promise<any> => {
    if(!status) return deleteProcess(targetName)
    return startCustomProcess(targetName, params)
}
