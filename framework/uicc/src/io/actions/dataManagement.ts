import fse from 'fs-extra';
import { KF_DATASET_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { listDir, removeFileFolder } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';


const path = require('path');

export const makeDataSeriesDir = (dirname: string) => {
    return fse.ensureDir(path.join(KF_DATASET_DIR, dirname))
}

export const getDataSeriesIdFromDataset = () => {
    return listDir(KF_DATASET_DIR)
}

export const removeDataseriesFolder = (dirname: string) => {
    return removeFileFolder(path.join(KF_DATASET_DIR, dirname))
}