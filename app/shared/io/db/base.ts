
import { runSelectDB } from '__gUtils/dbUtils';
import { GLOBAL_COMMISSION_DB } from '__gConfig/pathConfig';

export const getFeeSettingData = () => {
    return runSelectDB(GLOBAL_COMMISSION_DB,`SELECT rowid, * FROM commission`)
}
