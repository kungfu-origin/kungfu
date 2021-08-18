import fse from 'fs-extra';
import path from 'path';
import { KF_HOME, KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH, KUNGFU_RESOURCES_DIR } from '__gConfig/pathConfig';
import { addFileSync } from '__gUtils/fileUtils';

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


export const copyKungfuKey = () => {
    const targetKeyFile = path.join(KF_HOME, 'kungfu.key');
    if (!fse.existsSync(targetKeyFile)) {
        fse.copyFileSync(path.join(KUNGFU_RESOURCES_DIR, 'key', 'kungfu.key'), targetKeyFile)
    }
}