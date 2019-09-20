import { deleteProcess, killKfc, startMaster, startLedger } from '__gUtils/processUtils';
import { delaySeconds } from '__gUtils/busiUtils';

export const switchMaster = async (status: boolean): Promise<any> => {
    if(!status){
        try {
            await deleteProcess('master');
            await killKfc()
        } catch(err) {
            throw err
        }
    }
}

export const switchLedger = (status: boolean): Promise<any> => {
    if(!status) return deleteProcess('ledger')   
    return startLedger(false)
}
