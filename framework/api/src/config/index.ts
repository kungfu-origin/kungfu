import fse, { ensureFileSync } from 'fs-extra';
import {
  KF_CONFIG_DEFAULT_PATH,
  KF_CONFIG_PATH,
  KF_INSTRUMENTS_DEFAULT_PATH,
  KF_INSTRUMENTS_PATH,
} from './pathConfig';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { mergeObject } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

export const initKfConfig = () => {
  if (!fse.existsSync(KF_CONFIG_PATH)) {
    ensureFileSync(KF_CONFIG_PATH);
    let kfConfigJSON = fse.readJsonSync(KF_CONFIG_DEFAULT_PATH);
    const kfConfigInitValue =
      readRootPackageJsonSync()?.appConfig?.kfConfigInitValue || {};
    kfConfigJSON = mergeObject(kfConfigJSON, kfConfigInitValue);
    fse.outputJsonSync(KF_CONFIG_PATH, kfConfigJSON);
  }
};

export const initKfDefaultInstruments = () => {
  if (!fse.existsSync(KF_INSTRUMENTS_PATH)) {
    ensureFileSync(KF_INSTRUMENTS_PATH);
    const kfInstrumentsJSON = fse.readJsonSync(KF_INSTRUMENTS_DEFAULT_PATH);
    fse.outputJsonSync(KF_INSTRUMENTS_PATH, kfInstrumentsJSON);
  }
};
