import os from 'os';
import { promisified as regedit, RegistryItem } from 'regedit';
import { VCDepsWinRegMap } from '../config/VCDepsConfig';

const KUNGFU_SAFE_CPUS_NUM = 4;

export const isWindows = () => os.platform() === 'win32';
export const isMacos = () => os.platform() === 'darwin';
export const isLinux = () => os.platform() === 'linux';

export const getCpusNum = () =>
  Promise.resolve(os.cpus() ? os.cpus().length : 1);

export const checkIfCpusNumSafe = () => {
  return getCpusNum().then((cpusNum) => {
    return Promise.resolve(cpusNum > KUNGFU_SAFE_CPUS_NUM);
  });
};

export const getWinRegistry = (keys: string[]) => {
  if (!isWindows()) return Promise.resolve(null);

  return regedit.list(keys);
};

export const isWinRegistryExists = (registry: RegistryItem) => {
  return !!registry && registry.exists;
};

export const checkVCDepsByVersion = async (
  version: KungfuApi.VCDepsVersionTypes,
) => {
  const versionDepKeys = VCDepsWinRegMap[version];

  if (!versionDepKeys || !versionDepKeys.length) return true;

  const registries = await getWinRegistry(versionDepKeys);
  if (!isWindows() && registries === null) return true;

  if (registries) {
    return versionDepKeys.some((key) => {
      return registries[key]?.exists;
    });
  }

  return false;
};
