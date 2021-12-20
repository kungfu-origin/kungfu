import fse, { ensureFileSync } from 'fs-extra';
import {
    KF_CONFIG_DEFAULT_PATH,
    KF_CONFIG_PATH,
    KF_HOME,
    KUNGFU_RESOURCES_DIR,
} from './pathConfig';
import path from 'path';

export const initKfConfig = () => {
    if (!fse.existsSync(KF_CONFIG_PATH)) {
        ensureFileSync(KF_CONFIG_PATH);
        const kfConfigJSON = fse.readJsonSync(KF_CONFIG_DEFAULT_PATH);
        fse.outputJsonSync(KF_CONFIG_PATH, kfConfigJSON);
    }
};

export const ensureKungfuKey = () => {
    const targetKeyFile = path.join(KF_HOME, 'kungfu.key');
    if (!fse.existsSync(targetKeyFile)) {
        fse.copyFileSync(
            path.join(KUNGFU_RESOURCES_DIR, 'key', 'kungfu.key'),
            targetKeyFile,
        );
    }
};
