import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  delayMilliSeconds,
  getProcessIdByKfLocation,
  kfLogger,
  removeJournal,
  setTimerPromiseTask,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  pm2KillGodDaemon,
  killExtra,
  killKfc,
  pm2Kill,
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetailData,
  startArchiveMakeTask,
  startGetProcessStatus,
  killKungfu,
  startMaster,
  startLedger,
  startDzxy,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { combineLatest, Observable } from 'rxjs';
import { ProcessListItem } from 'src/typings';
import colors from 'colors';
import { Widgets } from 'blessed';
import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { dealStatus } from '../methods/utils';

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

export const processListObservable = () =>
  combineLatest(
    mdTdStrategyObservable(),
    processStatusDataObservable(),
    (
      mdTdStrategy: Record<KfCategoryTypes, KungfuApi.KfConfig[]>,
      ps: {
        processStatus: Pm2ProcessStatusData;
        processStatusWithDetail: Pm2ProcessStatusDetailData;
      },
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
          statusName: dealStatus(processStatus[processId] || '--'),
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
          statusName: dealStatus(processStatus[processId] || '--'),
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
          processName: colors.bold('_archive_'),
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
          processName: colors.bold('MASTER'),
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
          processName: colors.bold('LEDGER'),
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
          processName: colors.bold('DZXY'),
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

  switch (proc.category) {
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
              `${startOrStop} Master process success!`,
              2,
              (err) => {
                if (err) {
                  console.error(err);
                }
              },
            );
          })
          .catch((err: Error) => kfLogger.error(err.message));
      }

      break;
  }
};

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
        return message.log(`Archive success!`, 2, (err) => {
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

setTimerPromiseTask((): Promise<void> => {
  return new Promise((resolve) => {
    process.send({
      type: 'process:msg',
      data: {
        type: 'REQ_APP_STATE',
      },
    });
    resolve();
  });
});
