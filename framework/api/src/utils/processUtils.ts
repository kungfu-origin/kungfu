import path from 'path';
import fse from 'fs-extra';
import os from 'os';
import fkill from 'fkill';
import { Proc, ProcessDescription, StartOptions } from 'pm2';
import pm2 from './pm2Custom';
import { getUserLocale } from 'get-user-locale';
import find from 'find-process';

import {
  kfLogger,
  dealSpaceInPath,
  setTimerPromiseTask,
  flattenExtensionModuleDirs,
  getProcessIdByKfLocation,
  getIfProcessRunning,
  getIfProcessDeleted,
  delayMilliSeconds,
  isTdMdStrategy,
} from '../utils/busiUtils';
import {
  buildProcessLogPath,
  EXTENSION_DIRS,
  KFC_DIR,
  KF_HOME,
  KF_RUNTIME_DIR,
} from '../config/pathConfig';
import { getKfGlobalSettingsValue } from '../config/globalSettings';
import { Observable } from 'rxjs';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { Pm2StartOptions } from '../typings/global';
const { t } = VueI18n.global;

process.env.PM2_HOME = path.resolve(os.homedir(), '.pm2');
const numCPUs = os.cpus() ? os.cpus().length : 1;
const isWin = os.platform() === 'win32';
const isLinux = os.platform() === 'linux';
const locale = getUserLocale().replace(/-/g, '_');

interface FindProcessResult {
  pid: number;
  ppid?: number;
  uid?: number;
  gid?: number;
  name: string;
  cmd?: string;
  username?: string;
}

export const findProcessByKeyword = (
  processName: string,
): Promise<FindProcessResult[]> => {
  const userid = os.userInfo().uid;
  return find('name', processName, true).then((processList) => {
    return processList.filter((item) => {
      return item.uid ? item.uid == userid : true;
    });
  });
};

export const findProcessByKeywordsByFindProcess = (
  tasks: string[],
): Promise<FindProcessResult[]> => {
  return Promise.all(tasks.map((key) => findProcessByKeyword(key))).then(
    (results) => {
      return results.reduce((pre, processList) => {
        pre = [...pre, ...processList];
        return pre;
      }, []);
    },
  );
};

export const findProcessByKeywordsByTaskList = (
  tasks: string[],
): Promise<FindProcessResult[]> => {
  const username = os.userInfo().username;
  const tasklist = require('tasklist');
  return tasklist({ verbose: true }).then((processList) => {
    return processList
      .filter((item) => tasks.indexOf(item.imageName) !== -1)
      .filter((item) => (item.username || '').split('\\')[1] == username)
      .map((item) => {
        return {
          pid: item.pid,
          name: item.imageName,
          username: item.username,
        };
      });
  });
};

export const findProcessByKeywords = (
  tasks: string[],
): Promise<FindProcessResult[]> => {
  return isWin
    ? findProcessByKeywordsByTaskList(tasks)
    : findProcessByKeywordsByFindProcess(tasks);
};

export const forceKill = (tasks: string[]): Promise<void> => {
  return findProcessByKeywords(tasks).then((processList) => {
    const pids = processList.map((item) => item.pid);

    console.log('Target to force kill processList ', processList);
    return fkill(pids, {
      force: true,
      tree: isWin ? true : false,
      ignoreCase: true,
      silent: process.env.NODE_ENV === 'development' ? true : false,
    }).catch((err) => {
      console.warn((<Error>err).message);
    });
  });
};

const kfcName = isWin ? 'kfc.exe' : 'kfc';

export const killKfc = (): Promise<void> =>
  new Promise((resolve) => {
    forceKill([kfcName]).finally(() => resolve());
  });

export const killKungfu = () => {
  if (isLinux) {
    return forceKill(['kungfu']);
  }

  return Promise.resolve(true);
};

export const killExtra = () => forceKill([kfcName, 'pm2']);

//===================== pm2 start =======================

export type Pm2ProcessStatusTypes =
  | 'online'
  | 'stopping'
  | 'stopped'
  | 'launching'
  | 'errored'
  | 'waiting restart'
  | 'one-launch-status';

export type Pm2ProcessStatusData = Record<
  string,
  Pm2ProcessStatusTypes | undefined
>;

export interface Pm2ProcessStatusDetail {
  monit: ProcessDescription['monit'];
  pid: ProcessDescription['pid'];
  name: ProcessDescription['name'];
  pm_id: ProcessDescription['pm_id'];

  status: Pm2Env['status'];
  created_at: Pm2Env['created_at'];
  script: Pm2Env['script'];
  cwd: Pm2Env['pm_cwd'];
  config_settings: string;
  args: string[];
}

export type Pm2ProcessStatusDetailData = Record<string, Pm2ProcessStatusDetail>;

export interface Pm2Env {
  pm_cwd?: string;
  pm_out_log_path?: string;
  pm_err_log_path?: string;
  exec_interpreter?: string;
  pm_uptime?: number;
  unstable_restarts?: number;
  restart_time?: number;
  status?: Pm2ProcessStatusTypes;
  instances?: number | 'max';
  pm_exec_path?: string;
  created_at?: number;
  script?: StartOptions['script'];
  args?: StartOptions['args'];
  env?: StartOptions['env'];
}

export interface Pm2Packet {
  process: {
    pm_id: number;
  };
  data: {
    type: string;
    body: Record<string, string | number | boolean>;
  };
}

export interface Pm2PacketMain {
  type: string;
  topic: string;
  data: object;
  id: number;
}

export interface Pm2Bus {
  on(type: string, cb: (packet: Pm2Packet) => void): void;
  // on(type: string, cb: (packet: Pm2Packet) => void): void;
}

export const pm2Connect = (): Promise<void> => {
  return new Promise((resolve, reject) => {
    pm2.connect((err: Error) => {
      if (err) {
        kfLogger.error(err);
        reject(err);
        return;
      }

      resolve();
    });
  });
};

export const pm2List = (): Promise<ProcessDescription[]> => {
  return new Promise((resolve, reject) => {
    pm2.list((err: Error, pList: ProcessDescription[]) => {
      if (err) {
        kfLogger.error(err);
        reject(err);
        return;
      }

      resolve(pList);
    });
  });
};

export const pm2Describe = (
  processId: string,
): Promise<ProcessDescription[]> => {
  return new Promise((resolve, reject) => {
    //此处无需connect, 不然windows会卡死
    pm2.describe(processId, (err: Error, pList: ProcessDescription[]) => {
      if (err) {
        kfLogger.error(err);
        reject(err);
        return;
      }

      resolve(pList);
    });
  });
};

const pm2Start = (options: Pm2StartOptions): Promise<Proc> => {
  return new Promise((resolve, reject) => {
    pm2Connect()
      .then(() => {
        pm2.start(options, (err: Error, proc: Proc) => {
          if (err) {
            kfLogger.error(err);
            reject(err);
            return;
          }

          resolve(proc);
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err);
        reject(err);
      });
  });
};

const pm2Stop = (processId: string): Promise<void> => {
  return new Promise((resolve, reject) => {
    pm2Connect()
      .then(() => {
        pm2.stop(processId, (err: Error) => {
          if (err) {
            kfLogger.error(err);
            reject(err);
            return;
          }

          resolve();
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err);
        reject(err);
      });
  });
};

const pm2Delete = (processId: string): Promise<void> => {
  return new Promise((resolve, reject) => {
    pm2Connect()
      .then(() => {
        pm2.delete(processId, (err: Error) => {
          if (err) {
            kfLogger.error(err);
            reject(err);
            return;
          }

          resolve();
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err);
        reject(err);
      });
  });
};

declare module 'pm2' {
  function kill(cb: (err: Error, res: { success: boolean }) => void): void;
}

export const pm2Kill = (): Promise<void> => {
  kfLogger.info('Pm2 Kill All');
  return new Promise((resolve, reject) => {
    pm2.kill(
      (
        err: Error,
        res: {
          success: boolean;
        },
      ) => {
        pm2.disconnect();
        if (err) {
          kfLogger.error(err);
          reject(err);
          return;
        }

        if (res.success) {
          resolve();
        } else {
          reject(new Error('pm2Kill res.success not true'));
        }
      },
    );
  });
};

export const pm2KillGodDaemon = (): Promise<void> => {
  kfLogger.info('Kill Pm2 God Daemon');
  return new Promise((resolve, reject) => {
    pm2Connect().then(() => {
      // the doc said required disconnect, but if use the disconnect, the daemon won't quit
      pm2.killDaemon((err: Error) => {
        pm2.disconnect();
        if (err) {
          kfLogger.error(err);
          reject(err);
          return;
        }

        resolve();
      });
    });
  });
};

export const pm2Disconnect = pm2.disconnect;

export function pm2LaunchBus(cb: (err: Error, pm2_bus: Pm2Bus) => void) {
  pm2.launchBus(cb);
}

export const startProcess = async (
  options: Pm2StartOptions,
): Promise<Proc | void> => {
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const optionsResolved: Pm2StartOptions = {
    name: options.name,
    args: options.args, //有问题吗？
    cwd: options.cwd || path.join(KFC_DIR),
    script: options.script || kfcName,
    interpreter: options.interpreter || 'none',
    output: buildProcessLogPath(options.name),
    error: buildProcessLogPath(options.name),
    merge_logs: true,
    log_date_format: 'YYYY-MM-DD HH:mm:ss',
    autorestart: options.autorestart || false,
    max_restarts: options.max_restarts || 1,
    min_uptime: 3600000, //该时间段内最大启动次数max_restarts, 如果超过则不重启, 如果没超过, 则一直重启
    restart_delay: 1000,
    watch: options.watch || false,
    force: options.force || false,
    exec_mode: 'fork',
    kill_timeout: options.kill_timeout || 16000,
    env: {
      RELOAD_AFTER_CRASHED: process.env.RELOAD_AFTER_CRASHED || 'false',
      EXTENSION_DIRS: extDirs
        .map((dir) => dealSpaceInPath(path.dirname(dir)))
        .join(path.delimiter),
      KFC_DIR: process.env.KFC_DIR || '',
      CLI_DIR: process.env.CLI_DIR || '',
      KF_HOME: dealSpaceInPath(KF_HOME),
      KF_RUNTIME_DIR: dealSpaceInPath(KF_RUNTIME_DIR),
      LANG: `${locale}.UTF-8`,
      PYTHONUTF8: '1',
      PYTHONIOENCODING: 'utf8',

      KFC_AS_VARIANT: '',
      ...options.env,

      // cover father process env
      APP_TYPE: '',
      APP_ID: '',
      UI_EXT_TYPE: '',
      BY_PASS_ACCOUNTING: '',
      BY_PASS_TRADINGDATA: '',
      BY_PASS_RESTORE: '',
    },
  };

  return pm2Start(optionsResolved).catch((err) => {
    kfLogger.error(err);
  });
};

export const stopProcess = pm2Stop;

export const requestStop = (
  watcher: KungfuApi.Watcher,
  kfLocation: KungfuApi.KfLocation,
) => {
  return Promise.resolve(watcher.requestStop(kfLocation));
};

export const graceStopProcess = async (
  watcher: KungfuApi.Watcher | null,
  kfLocation: KungfuApi.KfConfig | KungfuApi.KfLocation,
  processStatusData?: Pm2ProcessStatusData,
): Promise<void> => {
  if (!processStatusData) {
    const { processStatus } = await listProcessStatus();
    processStatusData = processStatus;
  }

  const processId = getProcessIdByKfLocation(kfLocation);

  if (!watcher) {
    return Promise.reject(new Error('Watcher is NULL'));
  }

  if (getIfProcessRunning(processStatusData, processId)) {
    if (
      watcher &&
      !watcher.isReadyToInteract(kfLocation) &&
      isTdMdStrategy(kfLocation.category)
    ) {
      return Promise.reject(new Error(t('未就绪', { processId })));
    }

    return requestStop(watcher, kfLocation)
      .then(() => delayMilliSeconds(200))
      .then(() => stopProcess(processId));
  }

  return Promise.resolve();
};

export const deleteProcess = pm2Delete;

export const graceDeleteProcess = async (
  watcher: KungfuApi.Watcher | null,
  kfLocation: KungfuApi.KfConfig | KungfuApi.KfLocation,
  processStatusData?: Pm2ProcessStatusData,
): Promise<void> => {
  if (!processStatusData) {
    const { processStatus } = await listProcessStatus();
    processStatusData = processStatus;
  }

  const processId = getProcessIdByKfLocation(kfLocation);

  if (!watcher) {
    return Promise.reject(new Error('Watcher is NULL'));
  }

  if (getIfProcessRunning(processStatusData, processId)) {
    if (
      watcher &&
      !watcher.isReadyToInteract(kfLocation) &&
      isTdMdStrategy(kfLocation.category)
    ) {
      return Promise.reject(new Error(t('未就绪', { processId })));
    }

    return requestStop(watcher, kfLocation)
      .then(() => delayMilliSeconds(1000))
      .then(() => deleteProcess(processId));
  } else if (!getIfProcessDeleted(processStatusData, processId)) {
    return deleteProcess(processId);
  } else {
    return Promise.resolve();
  }
};

export function startProcessGetStatusUntilStop(
  options: Pm2StartOptions,
  cb?: (processStatus: Pm2ProcessStatusTypes) => void,
) {
  return new Promise((resolve) => {
    startProcess({ ...options }).then(() => {
      const timer = startGetProcessStatusByName(
        options.name,
        (res: ProcessDescription[]) => {
          const status = res[0]?.pm2_env?.status as Pm2ProcessStatusTypes;
          cb && cb(status);
          if (status !== 'online') {
            timer.clearLoop();
            resolve(status);
          }
        },
      );
    });
  });
}

export const startGetProcessStatus = (
  callback: (res: {
    processStatus: Pm2ProcessStatusData;
    processStatusWithDetail: Pm2ProcessStatusDetailData;
  }) => void,
) => {
  setTimerPromiseTask(() => {
    return listProcessStatus()
      .then((res) => {
        const { processStatus, processStatusWithDetail } = res;
        if (callback) {
          callback({
            processStatus: Object.freeze(processStatus || {}),
            processStatusWithDetail: Object.freeze(
              processStatusWithDetail || {},
            ),
          } as {
            processStatus: Pm2ProcessStatusData;
            processStatusWithDetail: Pm2ProcessStatusDetailData;
          });
        }
      })
      .catch((err) => console.error(err));
  }, 1000);
};

export const listProcessStatus = (): Promise<{
  processStatus: Pm2ProcessStatusData;
  processStatusWithDetail: Pm2ProcessStatusDetailData;
}> => {
  return pm2List().then((pList: ProcessDescription[]) => {
    const processStatus = buildProcessStatus(pList);
    const processStatusWithDetail = buildProcessStatusWidthDetail(pList);
    return { processStatus, processStatusWithDetail };
  });
};

export const listProcessStatusWithDetail =
  (): Promise<Pm2ProcessStatusDetailData> => {
    return pm2List().then((pList: ProcessDescription[]) =>
      buildProcessStatusWidthDetail(pList),
    );
  };

export function buildProcessStatusWidthDetail(
  pList: ProcessDescription[],
): Pm2ProcessStatusDetailData {
  return pList.reduce(
    (processStatus: Pm2ProcessStatusDetailData, p: ProcessDescription) => {
      const { monit, pid, name, pm2_env, pm_id } = p;
      const status = pm2_env?.status;
      const created_at = (pm2_env as Pm2Env)?.created_at;
      const cwd = pm2_env?.pm_cwd;
      const config_settings =
        (pm2_env as Pm2Env)?.env?.['CONFIG_SETTING'] || '[]';
      const pm_exec_path = (pm2_env?.pm_exec_path || '').split('/');
      const script =
        (pm2_env as Pm2Env).script || pm_exec_path[pm_exec_path.length - 1];
      const args = (pm2_env as Pm2Env).args || [];
      const argsResolved = typeof args === 'string' ? [args] : args;

      if (!name) return processStatus;
      processStatus[name] = {
        monit,
        pid,
        name,
        pm_id,
        status,
        created_at,
        script,
        cwd,
        args: argsResolved,
        config_settings,
      };
      return processStatus;
    },
    {} as Pm2ProcessStatusDetailData,
  );
}

//===================== pm2 end =========================

//===================== utils start =====================

function buildProcessStatus(pList: ProcessDescription[]): Pm2ProcessStatusData {
  return pList.reduce((pre, p: ProcessDescription) => {
    const name: string | undefined = p?.name;
    const status: Pm2ProcessStatusTypes | undefined = p?.pm2_env?.status;
    if (name) {
      pre[name] = status;
    }
    return pre;
  }, {} as Pm2ProcessStatusData);
}

function getRocketParams(args: string, ifRocket: boolean) {
  let rocket = ifRocket ? '-x' : '';

  if (numCPUs <= 4) {
    rocket = '';
  }

  if (args.includes('archive')) {
    rocket = '';
  }

  if (args.includes('dzxy')) {
    rocket = '';
  }

  return rocket;
}

function buildArgs(args: string): string {
  const globalSetting = getKfGlobalSettingsValue();
  const logLevel: string = globalSetting?.system?.logLevel ?? '';
  const ifRocket = globalSetting?.performance?.rocket ?? false;
  const rocket = getRocketParams(args, ifRocket);
  return [logLevel, args, rocket].join(' ');
}

//循环获取processStatus
function startGetProcessStatusByName(
  name: string,
  callback: (pList: ProcessDescription[]) => void,
) {
  const timer = setTimerPromiseTask(() => {
    return pm2Describe(name)
      .then((pList: ProcessDescription[]) => {
        callback(pList);
      })
      .catch((err) => kfLogger.error(err));
  }, 1000);

  return timer;
}

//===================== utils end =======================

//================ business related start ===============

export function startArchiveMakeTask(
  cb?: (processStatus: Pm2ProcessStatusTypes) => void,
) {
  const globalSetting = getKfGlobalSettingsValue();
  const bypassArchive = globalSetting?.system?.bypassArchive ?? false;
  return startProcessGetStatusUntilStop(
    {
      name: 'archive',
      args: buildArgs(`journal archive ${bypassArchive ? '-m delete' : ''}`),
    },
    cb,
  );
}

export const startMaster = async (force = false): Promise<void> => {
  const processName = 'master';

  try {
    await preStartProcess(processName, force);
    if (force) await killKfc();
    const args = buildArgs('run -c system -g master -n master');
    await startProcess({
      name: processName,
      args,
      force,
      env: {
        KF_NO_EXT: 'on',
      },
    });
  } catch (err: unknown) {
    kfLogger.error((<Error>err).message);
  }
};

//启动ledger
export const startLedger = async (force = false): Promise<void> => {
  const processName = 'ledger';

  try {
    await preStartProcess(processName, force);
    const args = buildArgs('run -c system -g service -n ledger');
    await startProcess({
      name: processName,
      args,
      force,
    });
  } catch (err: unknown) {
    kfLogger.error((<Error>err).message);
  }
};

export const startCacheD = async (force = false): Promise<void> => {
  const processName = 'cached';

  try {
    await preStartProcess(processName, force);
    const args = buildArgs('run -c system -g service -n cached');
    await startProcess({
      name: processName,
      args,
      force,
    });
  } catch (err: unknown) {
    kfLogger.error((<Error>err).message);
  }
};

async function preStartProcess(
  processName: string,
  force = false,
): Promise<void> {
  const processOrError = await pm2Describe(processName);
  if (processOrError instanceof Error) {
    kfLogger.error(processOrError.message);
    throw processOrError;
  }

  const isProcessAlive = !!processOrError.filter(
    (item) => item?.pm2_env?.status === 'online',
  ).length;

  if (!force && isProcessAlive) {
    const err = new Error(`kungfu ${processName} is alive`);
    kfLogger.error(err);
    return Promise.reject(err);
  }

  return Promise.resolve();
}

//启动md
export const startMd = async (
  sourceId: string,
  kfConfig: KungfuApi.DerivedKfLocation,
): Promise<Proc | void> => {
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const args = buildArgs(
    `-X "${extDirs
      .map((dir) => dealSpaceInPath(path.dirname(dir)))
      .join(path.delimiter)}" run -c md -g "${sourceId}" -n "${sourceId}"`,
  );
  const cwd = dealSpaceInPath(
    path.join(KF_RUNTIME_DIR, 'md', sourceId, sourceId),
  );
  await fse.ensureDir(cwd);
  const options =
    await globalThis.HookKeeper.getHooks().resolveStartOptions.trigger(
      kfConfig,
      {
        name: `md_${sourceId}`,
        cwd,
        script: `${dealSpaceInPath(path.join(KFC_DIR, kfcName))}`,
        args,
        max_restarts: 3,
        autorestart: true,
        force: true,
      },
    );

  return startProcess(options).catch((err) => {
    kfLogger.error(err);
  });
};

//启动td
export const startTd = async (
  accountId: string,
  kfConfig: KungfuApi.DerivedKfLocation,
): Promise<Proc | void> => {
  const globalSetting = getKfGlobalSettingsValue();
  const autorestart = globalSetting?.system?.autoRestartTd ?? true;
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const { source, id } = (accountId || '').parseSourceAccountId();
  const args = buildArgs(
    `-X "${extDirs
      .map((dir) => dealSpaceInPath(path.dirname(dir)))
      .join(path.delimiter)}" run -c td -g "${source}" -n "${id}"`,
  );
  const cwd = dealSpaceInPath(path.join(KF_RUNTIME_DIR, 'td', source, id));
  await fse.ensureDir(cwd);
  const fullProcessId = `td_${accountId}`;
  const options =
    await globalThis.HookKeeper.getHooks().resolveStartOptions.trigger(
      kfConfig,
      {
        name: fullProcessId,
        cwd,
        script: `${dealSpaceInPath(path.join(KFC_DIR, kfcName))}`,
        args,
        ...(autorestart
          ? {
              max_restarts: 4, // pm2 在进程退出时对重启次数进行 +1，所有第一次退出也被计算在内，重启 3 次的话这里就应该填 4
              autorestart: true,
            }
          : {}),
        force: true,
      },
    );

  return startProcess(options).catch((err) => {
    kfLogger.error(err);
  });
};

export const startTask = async (
  taskLocation: KungfuApi.KfLocation,
  soPath: string,
  args: string,
  configSettings: KungfuApi.KfConfigItem[],
): Promise<Proc | void> => {
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const argsResolved: string = buildArgs(
    `-X "${extDirs
      .map((dir) => dealSpaceInPath(path.dirname(dir)))
      .join(path.delimiter)}" run -c strategy -g "${taskLocation.group}" -n "${
      taskLocation.name
    }" '${soPath}' -a '${args}'`, // args is a JSON string
  );

  return startProcess({
    name: getProcessIdByKfLocation(taskLocation),
    args: argsResolved,
    env: {
      CONFIG_SETTING: JSON.stringify(configSettings),
    },
    force: true,
  }).catch((err) => {
    kfLogger.error(err);
  });
};

export const startStrategyByLocalPython = async (
  name: string,
  strategyPath: string,
  pythonPath: string,
): Promise<Proc | void> => {
  const baseArgs = [
    'run',
    '-c',
    'strategy',
    '-g',
    'default',
    '-n',
    name,
    `'${strategyPath}'`,
  ].join(' ');
  const baseArgsResolved = buildArgs(baseArgs);
  const args = ['-m', 'kungfu', baseArgsResolved].join(' ');

  if (!pythonPath.trim()) {
    return Promise.reject(new Error('No local python path!'));
  }

  const fullPythonPathList = pythonPath.replace(/\\/g, '/').split('/');
  const pythonFolder = fullPythonPathList
    .slice(0, fullPythonPathList.length - 1)
    .join('/');
  const pythonFile = fullPythonPathList
    .slice(fullPythonPathList.length - 1)
    .join('/');

  return startProcess({
    name: `strategy_${name}`,
    args,
    cwd: `${dealSpaceInPath(pythonFolder)}`,
    script: `${pythonFile}`,
    force: true,
  }).catch((err) => {
    kfLogger.error(err);
  });
};

//启动strategy
export const startStrategy = async (
  strategyId: string,
  strategyPath: string,
): Promise<Proc | void> => {
  strategyPath = dealSpaceInPath(strategyPath);
  const globalSetting = getKfGlobalSettingsValue();
  const ifLocalPython = globalSetting?.strategy?.python ?? false;
  const pythonPath = globalSetting?.strategy?.pythonPath ?? '';
  const strategyIdResolved = `strategy_${strategyId}`;

  //因为pm2环境残留，在反复切换本地python跟内置python时，会出现本地python启动失败，所以需要先pm2 kill
  try {
    kfLogger.info(`Clear existed strategy ${strategyIdResolved}`);
    await deleteProcess(strategyIdResolved);
  } catch (err) {
    kfLogger.warn(err);
  }

  if (ifLocalPython && strategyPath.endsWith('.py')) {
    return startStrategyByLocalPython(strategyId, strategyPath, pythonPath);
  } else {
    const args = buildArgs(
      `run -c strategy -g default -n '${strategyId}' '${strategyPath}'`,
    );
    return startProcess({
      name: strategyIdResolved,
      args,
      force: true,
    }).catch((err) => {
      kfLogger.error(err);
    });
  }
};

export const startDzxy = () => {
  return startProcess({
    name: 'dzxy',
    args: '',
    cwd: process.env.CLI_DIR,
    script: 'dzxy.js',
    interpreter: path.join(KFC_DIR, kfcName),
    force: true,
    watch: process.env.NODE_ENV === 'production' ? false : true,
    env: {
      KFC_AS_VARIANT: 'node',
    },
    kill_timeout: 500,
  }).catch((err) => {
    kfLogger.error(err);
  });
};

export const startExtDaemon = (name: string, cwd: string, script: string) => {
  return startProcess({
    name,
    args: '',
    cwd,
    script,
    interpreter: path.join(KFC_DIR, kfcName),
    force: true,
    watch: process.env.NODE_ENV === 'production' ? false : true,
    env: {
      KFC_AS_VARIANT: 'node',
    },
    kill_timeout: 500,
  }).catch((err) => {
    kfLogger.error(err);
  });
};

export const startBar = (
  targetName: string,
  source: string,
  timeInterval: string,
): Promise<Proc | void> => {
  return startProcess({
    name: targetName,
    args: buildArgs(`service bar -s ${source} --time-interval ${timeInterval}`),
  }).catch((err) => {
    kfLogger.error(err);
  });
};

export const startCustomProcess = (
  targetName: string,
  params: string,
): Promise<Proc | void> => {
  const args = buildArgs(`${targetName} ${params}`);
  return startProcess({
    name: targetName,
    args,
  }).catch((err) => {
    kfLogger.error(err);
  });
};

export const sendDataToProcessIdByPm2 = (
  topic: string,
  pm2Id: number,
  data: Record<string, KungfuApi.KfConfigValue>,
): Promise<void> => {
  return new Promise((resolve, reject) => {
    pm2.sendDataToProcessId(
      pm2Id,
      {
        type: 'process:msg',
        topic,
        data,
      },
      (err) => {
        if (err) {
          reject(err);
        } else {
          resolve();
        }
      },
    );
  });
};

export const processStatusDataObservable = () => {
  return new Observable<{
    processStatus: Pm2ProcessStatusData;
    processStatusWithDetail: Pm2ProcessStatusDetailData;
  }>((observer) => {
    startGetProcessStatus(
      (res: {
        processStatus: Pm2ProcessStatusData;
        processStatusWithDetail: Pm2ProcessStatusDetailData;
      }) => {
        observer.next(res);
      },
    );
  });
};

//================ business related end =================
