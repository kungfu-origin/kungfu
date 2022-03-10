import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import {
  BrokerStateStatusTypes,
  KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  delayMilliSeconds,
  getProcessIdByKfLocation,
  kfLogger,
  removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  pm2KillGodDaemon,
  killExtra,
  killKfc,
  pm2Kill,
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetailData,
  sendDataToProcessIdByPm2,
  startArchiveMakeTask,
  startGetProcessStatus,
  killKungfu,
  startMaster,
  startLedger,
  startDzxy,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { combineLatest, filter, map, Observable, Subject } from 'rxjs';
import { ProcessListItem } from 'src/typings';
import colors from 'colors';
import { Widgets } from 'blessed';
import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { dealStatus } from '../methods/utils';
import { Pm2Bus, Pm2Packet } from '../../typings';
import pm2 from 'pm2';

const DZXY_SUBJECT = new Subject<Pm2Packet>();
let DZXY_PM_ID = 0;
let DZXY_WATCHER_IS_LIVE = false;

export const mdTdStrategyObservable = () => {
  return new Observable<Record<KfCategoryTypes, KungfuApi.KfConfig[]>>(
    (observer) => {
      getAllKfConfigOriginData().then(
        (allConfigs: Record<KfCategoryTypes, KungfuApi.KfConfig[]>) => {
          observer.next(allConfigs);
        },
      );
    },
  );
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

export const appStatesObservable = () => {
  return DZXY_SUBJECT.pipe(
    filter((packet: Pm2Packet) => {
      if (packet.data.type === 'APP_STATES') {
        return true;
      }
      return false;
    }),
    map((packet: Pm2Packet) => {
      return packet.data.body as Record<string, BrokerStateStatusTypes>;
    }),
  );
};

export const processListObservable = () =>
  combineLatest(
    mdTdStrategyObservable(),
    processStatusDataObservable(),
    appStatesObservable(),
    (
      mdTdStrategy: Record<KfCategoryTypes, KungfuApi.KfConfig[]>,
      ps: {
        processStatus: Pm2ProcessStatusData;
        processStatusWithDetail: Pm2ProcessStatusDetailData;
      },
      appStates: Record<string, BrokerStateStatusTypes>,
    ): ProcessListItem[] => {
      const { md, td, strategy } = mdTdStrategy;
      const { processStatus, processStatusWithDetail } = ps;

      const mdList: ProcessListItem[] = md.map((item) => {
        const processId = getProcessIdByKfLocation(item);
        return {
          processId,
          processName: processId,
          typeName: colors.yellow('Md'),
          category: item.category,
          group: item.group,
          name: item.name,
          value: JSON.parse(item.value || '{}'),
          status: processStatus[processId] || '--',
          statusName: dealStatus(
            processStatus[processId]
              ? appStates[processId] || processStatus[processId] || '--'
              : '--',
          ),
          monit: processStatusWithDetail[processId]?.monit,
        };
      });

      const tdList: ProcessListItem[] = td.map((item) => {
        const processId = getProcessIdByKfLocation(item);
        return {
          processId,
          processName: processId,
          typeName: colors.cyan('Td'),
          category: item.category,
          group: item.group,
          name: item.name,
          value: JSON.parse(item.value || '{}'),
          status: processStatus[processId] || '--',
          statusName: dealStatus(
            processStatus[processId]
              ? appStates[processId] || processStatus[processId] || '--'
              : '--',
          ),
          monit: processStatusWithDetail[processId]?.monit,
        };
      });

      const strategyList: ProcessListItem[] = strategy.map((item) => {
        const processId = getProcessIdByKfLocation(item);
        return {
          processId,
          processName: processId,
          typeName: colors.blue('Strat'),
          category: item.category,
          group: item.group,
          name: item.name,
          value: JSON.parse(item.value || '{}'),
          status: processStatus[processId] || '--',
          statusName: dealStatus(processStatus[processId] || '--'),
          monit: processStatusWithDetail[processId]?.monit,
        };
      });

      return [
        {
          processId: 'archive',
          processName: colors.gray('_archive_'),
          typeName: colors.bgMagenta('Sys'),
          category: 'system',
          group: '',
          name: '',
          value: {},
          status: processStatus['archive'] || '--',
          statusName: dealStatus(processStatus['archive'] || '--'),
          monit: processStatusWithDetail['archive']?.monit,
        },
        {
          processId: 'master',
          processName: colors.bold(colors.white('MASTER')),
          typeName: colors.bgMagenta('Sys'),
          category: 'system',
          group: 'master',
          name: 'master',
          value: {},
          status: processStatus['master'] || '--',
          statusName: dealStatus(processStatus['master'] || '--'),
          monit: processStatusWithDetail['master']?.monit,
        },
        {
          processId: 'ledger',
          processName: colors.gray('LEDGER'),
          typeName: colors.bgMagenta('Sys'),
          category: 'system',
          group: 'service',
          name: 'ledger',
          value: {},
          status: processStatus['ledger'] || '--',
          statusName: dealStatus(processStatus['ledger'] || '--'),
          monit: processStatusWithDetail['ledger']?.monit,
        },
        {
          processId: 'dzxy',
          processName: colors.grey('DZXY'),
          typeName: colors.bgMagenta('Sys'),
          category: 'system',
          group: 'service',
          name: 'dzxy',
          value: {},
          status: processStatus['dzxy'] || '--',
          statusName: dealStatus(processStatus['dzxy'] || '--'),
          monit: processStatusWithDetail['dzxy']?.monit,
        },
        ...mdList,
        ...tdList,
        ...strategyList,
      ];
    },
  );

export const switchProcess = (
  proc: ProcessListItem,
  messageBoard: Widgets.MessageElement,
  loading: Widgets.LoadingElement,
) => {
  const status = proc.status !== '--';
  const startOrStop = status ? 'Stop' : 'Start';
  const { category, group, name, value } = proc;

  switch (category) {
    case 'system':
      if (proc.processId === 'master') {
        //开启，要归档, cli 需要clearjournal
        preSwitchMain(status, messageBoard, loading)
          .then(() => {
            loading.load(`${startOrStop} Master process`);
            return switchMaster(!status);
          })
          .then(() => {
            loading.stop();
            return messageBoard.log(
              `${startOrStop} Master process success`,
              2,
              (err) => {
                if (err) {
                  console.error(err);
                }
              },
            );
          })
          .catch((err: Error) => kfLogger.error(err.message));
      } else {
        if (status) {
          messageBoard.log('Stop master first', 2, (err) => {
            if (err) {
              console.error(err);
            }
          });
        } else {
          messageBoard.log('Start master first', 2, (err) => {
            if (err) {
              console.error(err);
            }
          });
        }
      }
      break;
    case 'md':
    case 'td':
    case 'strategy':
      if (!DZXY_WATCHER_IS_LIVE) {
        messageBoard.log('Start master first', 2, (err) => {
          if (err) {
            console.error(err);
          }
        });
        return;
      }

      return sendDataToProcessIdByPm2('SWITCH_KF_LOCATION', DZXY_PM_ID, {
        category,
        group,
        name,
        value: JSON.stringify(value),
        status,
      })
        .then(() => {
          messageBoard.log('Please wait...', 2, (err) => {
            if (err) {
              console.error(err);
            }
          });
        })
        .catch((err) => {
          const errMsg = parseSwtchKfLocationErrMessage(err.message);
          messageBoard.log(errMsg, 2, (err) => {
            if (err) {
              console.error(err);
            }
          });
        });
  }
};

function parseSwtchKfLocationErrMessage(errMsg: string): string {
  if (errMsg.includes('offline')) {
    return 'Start master first, If did, Please wait...';
  }

  return errMsg;
}

function preSwitchMain(
  status: boolean,
  message: Widgets.MessageElement,
  loading: Widgets.LoadingElement,
) {
  if (!status) {
    loading.load(`Start Archive, Please wait...`);
    return removeJournal(KF_HOME)
      .then(() => startArchiveMakeTask())
      .then(() => {
        loading.stop();
        return message.log(`Archive success`, 2, (err) => {
          if (err) {
            console.error(err);
          }
        });
      });
  }

  return Promise.resolve(true);
}

const switchMaster = async (status: boolean): Promise<void> => {
  if (!status) {
    await pm2Kill();
    await pm2KillGodDaemon();
    await killKfc();
    await killExtra();
    if (process.env.NODE_ENV === 'production') {
      await killKungfu();
    }
  } else {
    await startMaster(false);
    await delayMilliSeconds(1000);
    await startLedger(false);
    await startDzxy();
  }
};

const timer = setTimeout(() => {
  DZXY_SUBJECT.next({
    process: {
      pm_id: -1,
    },
    data: {
      type: 'APP_STATES',
      body: {},
    },
  });
  clearTimeout(timer);
}, 1000);

pm2.launchBus((err: Error, pm2_bus: Pm2Bus) => {
  if (err) {
    console.error('pm2 launchBus Error', err);
    return;
  }

  pm2_bus.on('process:msg', (packet: Pm2Packet) => {
    const processData = packet.process;
    DZXY_PM_ID = processData.pm_id;

    if (packet.data.type === 'WATCHER_IS_LIVE') {
      DZXY_WATCHER_IS_LIVE = !!packet.data.body;
    }

    DZXY_SUBJECT.next(packet);
  });
});
