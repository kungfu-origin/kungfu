

import { KF_DATASET_DIR } from '__gConfig/pathConfig';
import { listDir } from '__gUtils/fileUtils';

const path = require('path');
const fse = require('fs-extra')


export const makeDataSeriesDir = (dirname: string) => {
    return fse.ensureDir(path.join(KF_DATASET_DIR, dirname))
}

export const getDataSeriesIdFromDataset = () => {
    return listDir(KF_DATASET_DIR)
}