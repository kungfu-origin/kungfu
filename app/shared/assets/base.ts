import fse from 'fs-extra';
import { KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { existsSync, addFileSync, readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils'

export const initConfig = () => {
    if(!existsSync(KF_CONFIG_PATH)) {
        addFileSync('', KF_CONFIG_PATH, 'file');
        const kfConfigJSON = readJsonSync(KF_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_CONFIG_PATH, kfConfigJSON)
    }

    if(!existsSync(KF_TARADING_CONFIG_PATH)) {
        addFileSync('', KF_TARADING_CONFIG_PATH, 'file');
        const kfTradingConfigJSON = readJsonSync(KF_TARADING_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_TARADING_CONFIG_PATH, kfTradingConfigJSON)
    }
};


