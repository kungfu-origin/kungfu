import { runSelectDB, runInsertUpdateDeleteDB } from '__gUtils/dbUtils';
import { buildDateRange } from '__gUtils/busiUtils';
import {
    STRATEGYS_DB, 
    LIVE_TRADING_DATA_DB
} from '__gConfig/pathConfig';
import moment from "moment"




export const getStrategyAssetById = (strategyId: string) => {
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        'SELECT * FROM asset' + 
        ' WHERE ledger_category = 1' +
        ` AND client_id = "${strategyId}"`
    )
}

