import fse from 'fs-extra';
import path from 'path';
import { KF_CONFIG_DIR } from '../config/pathConfig';
import { GlobalStorageData } from '../typings/global';

function initStorage() {
  try {
    const _storage = readStorage();
    if (_storage) {
      globalStorage._storage = _storage;
      return true;
    }
    fse.writeJSONSync(globalStorage._storageUrl, {});
    globalStorage._storage = {};
    return true;
  } catch (e) {
    return false;
  }
}

function readStorage(): GlobalStorageData | false {
  try {
    return fse.readJSONSync(globalStorage._storageUrl);
  } catch (error) {
    return false;
  }
}

function writeStorage(value: GlobalStorageData) {
  try {
    fse.writeJsonSync(globalStorage._storageUrl, value);
    return true;
  } catch (e) {
    return false;
  }
}

export type GlobalStorageDataKeys = keyof GlobalStorageData;

export interface GlobalStorage {
  _storage: GlobalStorageData;
  _storageUrl: string;
  setStoragePath: (path: string) => void;
  getStoragePath: () => string;
  getItem: <T extends GlobalStorageDataKeys>(key: T) => GlobalStorageData[T];
  setItem: <T extends GlobalStorageDataKeys>(
    key: T,
    value: Exclude<GlobalStorageData[T], undefined | null>,
  ) => boolean;
  getAll: () => GlobalStorageData;
  removeItem: (key: GlobalStorageDataKeys) => boolean;
  clear: () => boolean;
}

const globalStorage: GlobalStorage = {
  _storage: { ifNotFirstRunning: false },
  _storageUrl: path.join(KF_CONFIG_DIR, 'globalStorage.json'),
  setStoragePath: (path: string) => {
    globalStorage._storageUrl = path;
  },
  getStoragePath: () => {
    return globalStorage._storageUrl;
  },
  getItem: <T extends GlobalStorageDataKeys>(key: T) => {
    return globalStorage._storage[key];
  },
  setItem: <T extends GlobalStorageDataKeys>(
    key: T,
    value: GlobalStorageData[T],
  ) => {
    const _storage = Object.assign({}, globalStorage._storage);
    _storage[key] = value;
    const suc = writeStorage(_storage);
    if (suc) {
      globalStorage._storage = _storage;
      return true;
    }
    return false;
  },
  getAll: () => {
    return globalStorage._storage;
  },
  removeItem: (key: GlobalStorageDataKeys) => {
    const value = globalStorage.getItem(key);
    if (value) {
      const _storage = Object.assign({}, globalStorage._storage);
      delete _storage[key];
      const suc = writeStorage(_storage);
      if (suc) {
        globalStorage._storage = _storage;
        return true;
      }
    }
    return false;
  },
  clear: () => {
    const suc = writeStorage({});
    if (suc) {
      globalStorage._storage = {};
      return true;
    }
    return false;
  },
};

if (!globalThis.globalStorage) {
  initStorage();
  globalThis.globalStorage = globalStorage;
}

export default globalThis.globalStorage as GlobalStorage;
