import os from 'os';
import { promisified as regedit, RegistryItem } from 'regedit';

const KUNGFU_SAFE_CPUS_NUM = 4;

export const isWindows = () => os.platform() === 'win32';
export const isMacos = () => os.platform() === 'darwin';
export const isLinux = () => os.platform() === 'linux';

export const getCpusNum = (): Promise<number> =>
  Promise.resolve(os.cpus() ? os.cpus().length : 1);

export const checkIfCpusNumSafe = (): Promise<boolean> => {
  return getCpusNum().then((cpusNum) => {
    return Promise.resolve(cpusNum > KUNGFU_SAFE_CPUS_NUM);
  });
};

export const getWinRegistry = (
  keys: string[],
): Promise<{ [x: string]: RegistryItem } | null> => {
  if (!isWindows()) return Promise.resolve(null);

  return regedit.list(keys);
};

export const isWinRegistryExists = (registry: RegistryItem): boolean => {
  return !!registry && registry.exists;
};
