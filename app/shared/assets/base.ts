import fse from 'fs-extra';
import { KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { addFileSync } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils'

export const initConfig = () => {
    if(!fse.existsSync(KF_CONFIG_PATH)) {
        addFileSync('', KF_CONFIG_PATH, 'file');
        const kfConfigJSON = fse.readJsonSync(KF_CONFIG_DEFAULT_PATH);
        fse.outputJsonSync(KF_CONFIG_PATH, kfConfigJSON)
    }

    if(!fse.existsSync(KF_TARADING_CONFIG_PATH)) {
        addFileSync('', KF_TARADING_CONFIG_PATH, 'file');
        const kfTradingConfigJSON = fse.readJsonSync(KF_TARADING_CONFIG_DEFAULT_PATH);
        fse.outputJsonSync(KF_TARADING_CONFIG_PATH, kfTradingConfigJSON)
    }
};


