import { deleteProcess, killKfc, startMaster, startLedger } from '__gUtils/processUtils';
import { delayMiliSeconds } from '__gUtils/busiUtils';

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
