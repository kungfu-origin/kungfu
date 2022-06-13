import path from 'path';
import os from 'os';
import fkill from 'fkill';
import { Proc, ProcessDescription, StartOptions } from 'pm2';
import pm2 from './pm2Custom';
import { getUserLocale } from 'get-user-locale';
import psList, { ProcessDescriptor } from 'ps-list';

import {
  kfLogger,
  dealSpaceInPath,
  setTimerPromiseTask,
  flattenExtensionModuleDirs,
  getProcessIdByKfLocation,
  getIfProcessRunning,
  getIfProcessDeleted,
  delayMilliSeconds,
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
const { t } = VueI18n.global;

process.env.PM2_HOME = path.resolve(os.homedir(), '.pm2');
const numCPUs = os.cpus() ? os.cpus().length : 1;
const isWin = os.platform() === 'win32';
const isLinux = os.platform() === 'linux';
const locale = getUserLocale().replace(/-/g, '_');

export const findProcessByKeywords = (tasks: string[]): Promise<number[]> => {
  return psList().then((processes: ProcessDescriptor[]) => {
    return processes
      .filter((item) => {
        const name = item.name;
        const afterFiler = tasks.filter((task) =>
          name.toLowerCase().includes(task.toLowerCase()),
        );
        return afterFiler.length;
      })
      .map((item) => item.pid);
  });
};

export const forceKill = (tasks: string[]): Promise<void> => {
  return findProcessByKeywords(tasks).then((pids) => {
    return fkill(pids, {
      force: true,
      tree: isWin ? true : false,
      ignoreCase: true,
      silent: process.env.NODE_ENV === 'development' ? true : false,
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

interface Pm2StartOptions extends StartOptions {
  name: string;
  autorestart?: boolean;
}

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

export class Pm2Bus {
  on: (type: string, cb: (packet: Pm2Packet) => void) => void;
}

export const pm2Connect = (): Promise<void> => {
  return new Promise((resolve, reject) => {
    pm2.connect((err: Error) => {
      if (err) {
        kfLogger.error(err.message);
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
        kfLogger.error(err.message);
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
        kfLogger.error(err.message);
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
            kfLogger.error(err.message);
            reject(err);
            return;
          }

          resolve(proc);
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err.message);
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
            kfLogger.error(err.message);
            reject(err);
            return;
          }

          resolve();
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err.message);
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
            kfLogger.error(err.message);
            reject(err);
            return;
          }

          resolve();
        });
      })
      .catch((err: Error) => {
        kfLogger.error(err.message);
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
          kfLogger.error(err.message);
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
          kfLogger.error(err.message);
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

export const startProcess = (
  options: Pm2StartOptions,
): Promise<Proc | void> => {
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
    kill_timeout: 16000,
    env: {
      RELOAD_AFTER_CRASHED: process.env.RELOAD_AFTER_CRASHED || 'false',
      EXTENSION_DIRS: process.env.EXTENSION_DIRS || '',
      KFC_DIR: process.env.KFC_DIR || '',
      KF_HOME: dealSpaceInPath(KF_HOME),
      KF_RUNTIME_DIR: dealSpaceInPath(KF_RUNTIME_DIR),
      LANG: `${locale}.UTF-8`,
      PYTHONUTF8: '1',
      PYTHONIOENCODING: 'utf8',
      KFC_AS_VARIANT: '',
      ...options.env,
    },
  };

  return pm2Start(optionsResolved).catch((err) => kfLogger.error(err.message));
};

export const stopProcess = pm2Stop;

export const graceStopProcess = (
  watcher: KungfuApi.Watcher | null,
  kfLocation: KungfuApi.KfConfig | KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
): Promise<void> => {
  const processId = getProcessIdByKfLocation(kfLocation);
  if (getIfProcessRunning(processStatusData, processId)) {
    if (watcher && !watcher.isReadyToInteract(kfLocation)) {
      return Promise.reject(new Error(t('未就绪', { processId })));
    }

     return Promise.resolve(watcher.requestStop(kfLocation))
      .then(() => delayMilliSeconds(1000))
      .then(() => stopProcess(processId));
  }

  return Promise.resolve();
};

export const deleteProcess = pm2Delete;

export const graceDeleteProcess = (
  watcher: KungfuApi.Watcher | null,
  kfLocation: KungfuApi.KfConfig | KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
): Promise<void> => {
  const processId = getProcessIdByKfLocation(kfLocation);

  if (!watcher) return Promise.reject(new Error('Watcher is NULL'));

  if (getIfProcessRunning(processStatusData, processId)) {
    if (watcher && !watcher.isReadyToInteract(kfLocation)) {
      return Promise.reject(new Error(t('未就绪', { processId })));
    }

    return Promise.resolve(watcher.requestStop(kfLocation))
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
  cb?: Function,
) {
  return new Promise((resolve) => {
    startProcess({ ...options }).then(() => {
      let timer = startGetProcessStatusByName(options.name, (res: any[]) => {
        const status = res[0]?.pm2_env?.status as Pm2ProcessStatusTypes;
        cb && cb(status);
        if (status !== 'online') {
          timer.clearLoop();
          resolve(status);
        }
      });
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
  return pm2List().then((pList: any[]) => {
    const processStatus = buildProcessStatus(pList);
    const processStatusWithDetail = buildProcessStatusWidthDetail(pList);
    return { processStatus, processStatusWithDetail };
  });
};

export const listProcessStatusWithDetail =
  (): Promise<Pm2ProcessStatusDetailData> => {
    return pm2List().then((pList: any[]) =>
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
  const logLevel: string = globalSetting?.system?.logLevel || '';
  const ifRocket = globalSetting?.performance?.rocket || false;
  const rocket = getRocketParams(args, ifRocket);
  return [logLevel, args, rocket].join(' ');
}

//循环获取processStatus
function startGetProcessStatusByName(name: string, callback: Function) {
  const timer = setTimerPromiseTask(() => {
    return pm2Describe(name)
      .then((pList: ProcessDescription[]) => {
        callback(pList);
      })
      .catch((err) => kfLogger.error(err.message));
  }, 1000);

  return timer;
}

//===================== utils end =======================

//================ business related start ===============

export function startArchiveMakeTask(cb?: Function) {
  return startProcessGetStatusUntilStop(
    {
      name: 'archive',
      args: buildArgs('journal archive'),
    },
    cb,
  );
}

export const startMaster = async (force = false): Promise<void> => {
  const processName = 'master';

  try {
    await preStartProcess(processName, force);
    await killKfc();
    const args = buildArgs('run -c system -g master -n master');
    await startProcess({
      name: processName,
      args,
      force,
      env: {
        KF_NO_EXT: 'on',
      },
    });
  } catch (err) {
    kfLogger.error(err.message);
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
  } catch (err) {
    kfLogger.error(err.message);
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
  } catch (err) {
    kfLogger.error(err.message);
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
    kfLogger.error(err.message);
    return Promise.reject(err);
  }

  return Promise.resolve();
}

//启动md
export const startMd = async (sourceId: string): Promise<Proc | void> => {
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const args = buildArgs(
    `-X "${extDirs
      .map((dir) => dealSpaceInPath(path.dirname(dir)))
      .join(path.delimiter)}" run -c md -g "${sourceId}" -n "${sourceId}"`,
  );
  return startProcess({
    name: `md_${sourceId}`,
    args,
    max_restarts: 3,
    autorestart: true,
  }).catch((err) => {
    kfLogger.error(err.message);
  });
};

//启动td
export const startTd = async (accountId: string): Promise<Proc | void> => {
  const extDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const { source, id } = accountId.parseSourceAccountId();
  const args = buildArgs(
    `-X "${extDirs
      .map((dir) => dealSpaceInPath(path.dirname(dir)))
      .join(path.delimiter)}" run -c td -g "${source}" -n "${id}"`,
  );
  return startProcess({
    name: `td_${accountId}`,
    args,
    max_restarts: 3,
    autorestart: true,
  }).catch((err) => {
    kfLogger.error(err.message);
  });
};

export const startTask = async (
  taskLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
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
  }).catch((err) => {
    kfLogger.error(err.message);
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

  const fullPythonPathList = pythonPath.split('/');
  const pythonFolder = fullPythonPathList
    .slice(0, fullPythonPathList.length - 1)
    .join('/');
  const pythonFile = fullPythonPathList
    .slice(fullPythonPathList.length - 1)
    .join('/');

  return startProcess({
    name: `strategy_${name}`,
    args,
    cwd: `'${pythonFolder}'`,
    script: `'${pythonFile}'`,
  }).catch((err) => {
    kfLogger.error(err.message);
  });
};

//启动strategy
export const startStrategy = (
  strategyId: string,
  strategyPath: string,
): Promise<Proc | void> => {
  strategyPath = dealSpaceInPath(strategyPath);
  const globalSetting = getKfGlobalSettingsValue();
  const ifLocalPython = globalSetting?.strategy?.python || false;
  const pythonPath = globalSetting?.strategy?.pythonPath || '';

  if (ifLocalPython && strategyPath.endsWith('.py')) {
    return startStrategyByLocalPython(strategyId, strategyPath, pythonPath);
  } else {
    const args = buildArgs(
      `run -c strategy -g default -n '${strategyId}' '${strategyPath}'`,
    );
    return startProcess({
      name: `strategy_${strategyId}`,
      args,
    }).catch((err) => {
      kfLogger.error(err.message);
    });
  }
};

export const startDzxy = () => {
  return startProcess({
    name: 'dzxy',
    args: '',
    cwd:
      process.env.NODE_ENV === 'development'
        ? path.join(process.cwd(), 'dist', 'cli')
        : path.resolve(__dirname),
    script: 'dzxy.js',
    interpreter: path.join(KFC_DIR, kfcName),
    force: true,
    watch: process.env.NODE_ENV === 'production' ? false : true,
    env: {
      KFC_AS_VARIANT: 'node',
    },
  }).catch((err) => {
    kfLogger.error(err.message);
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
  }).catch((err) => {
    kfLogger.error(err.message);
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
    kfLogger.error(err.message);
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
    kfLogger.error(err.message);
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
