import path from 'path';
import fse from 'fs-extra';
import { getKfAllConfig, removeKfConfig } from '../kungfu/store';
import {
  KfCategoryEnum,
  KfCategoryTypes,
  KfModeEnum,
  KfModeTypes,
} from '../typings/enums';
import {
  KF_RUNTIME_DIR,
  KF_SCHEDULE_TASKS_JSON_PATH,
  KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH,
  KF_TD_GROUP_JSON_PATH,
  LOG_DIR,
} from '../config/pathConfig';
import { pathExists, remove } from 'fs-extra';
import {
  getIdByKfLocation,
  getProcessIdByKfLocation,
} from '../utils/busiUtils';
import {
  getAllKfRiskSettings,
  setAllKfRiskSettings,
} from '../kungfu/riskSetting';
import {
  graceDeleteProcess,
  Pm2ProcessStatusData,
} from '../utils/processUtils';

export const getAllKfConfigOriginData = (): Promise<
  Record<KfCategoryTypes, KungfuApi.KfConfig[]>
> => {
  return getKfAllConfig().then((allConfig: KungfuApi.KfConfigOrigin[]) => {
    const allConfigResolved = allConfig.map(
      (config: KungfuApi.KfConfigOrigin): KungfuApi.KfConfig => {
        return {
          ...config,
          category: KfCategoryEnum[config.category] as KfCategoryTypes,
          mode: KfModeEnum[config.mode] as KfModeTypes,
        };
      },
    );

    return {
      md: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
        return config.category === 'md';
      }),
      td: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
        return config.category === 'td';
      }),
      strategy: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
        return config.category === 'strategy';
      }),
      system: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
        return config.category === 'system';
      }),
      daemon: [],
    };
  });
};

export const isKfConfig = async (kfLocation: KungfuApi.KfLocation) => {
  const allConfig = await getAllKfConfigOriginData();
  const { category, group, name } = kfLocation;
  if (!allConfig[category]) {
    return false;
  }

  return (
    allConfig[category].findIndex(
      (item) => item.group === group && item.name === name,
    ) !== -1
  );
};

export const deleteAllByKfLocation = async (
  kfLocation: KungfuApi.KfLocation,
): Promise<void> => {
  const isConfig = await isKfConfig(kfLocation);

  return (isConfig ? removeKfConfig(kfLocation) : Promise.resolve())
    .then(() => removeKfLocation(kfLocation))
    .then(() => removeLog(kfLocation));
};

export const ensureRemoveLocation = (
  watcher: KungfuApi.Watcher,
  kfLocation: KungfuApi.KfConfig | KungfuApi.KfLocation,
  processStatus: Pm2ProcessStatusData,
) => {
  return graceDeleteProcess(watcher, kfLocation, processStatus).then(() =>
    deleteAllByKfLocation(kfLocation),
  );
};

export function removeKfLocation(
  kfLocation: KungfuApi.KfLocation,
): Promise<void> {
  const targetDir = path.resolve(
    KF_RUNTIME_DIR,
    kfLocation.category,
    kfLocation.group,
    kfLocation.name,
  );

  return pathExists(targetDir).then((isExisted: boolean): Promise<void> => {
    if (isExisted) {
      return remove(targetDir);
    }

    console.warn(`Location Dir ${targetDir} is not existed`);
    return Promise.resolve();
  });
}

export function removeLog(kfLocation: KungfuApi.KfLocation): Promise<void> {
  const logPath = path.resolve(
    LOG_DIR,
    `${getProcessIdByKfLocation(kfLocation)}.log`,
  );
  return pathExists(logPath).then((isExisted: boolean): Promise<void> => {
    if (isExisted) {
      return remove(logPath);
    }

    console.warn(`Log Path ${logPath} is not existed`);
    return Promise.resolve();
  });
}

export const getSubscribedInstruments = async (): Promise<
  KungfuApi.InstrumentResolved[]
> => {
  return fse.readFile(KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH).then((res) => {
    const str = Buffer.from(res).toString();
    return JSON.parse(str || '[]');
  });
};

export const addSubscribeInstruments = async (
  instruments: KungfuApi.InstrumentResolved[],
): Promise<void> => {
  const oldInstruments = await getSubscribedInstruments();
  return fse.outputJSON(KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH, [
    ...oldInstruments,
    ...instruments,
  ]);
};

export const removeSubscribeInstruments = async (
  instrument: KungfuApi.InstrumentResolved,
): Promise<void> => {
  const oldInstruments = await getSubscribedInstruments();
  const removeTargetIndex = oldInstruments.findIndex((item) => {
    if (item.instrumentId === instrument.instrumentId) {
      if (item.exchangeId === instrument.exchangeId) {
        return true;
      }
    }
    return false;
  });

  if (removeTargetIndex === -1) {
    return Promise.reject(
      new Error(
        `Not found instrument ${instrument.exchangeId}_${instrument.instrumentId} `,
      ),
    );
  }

  oldInstruments.splice(removeTargetIndex, 1);
  return fse.outputJSON(KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH, oldInstruments);
};

export const getTdGroups = async (): Promise<KungfuApi.KfExtraLocation[]> => {
  return fse.readFile(KF_TD_GROUP_JSON_PATH).then((res) => {
    const str = Buffer.from(res).toString();
    return JSON.parse(str || '[]');
  });
};

export const addTdGroup = async (
  tdGroup: KungfuApi.KfExtraLocation,
): Promise<void> => {
  const oldTdGroups = await getTdGroups();
  return fse.outputJSON(KF_TD_GROUP_JSON_PATH, [...oldTdGroups, tdGroup]);
};

export const removeTdGroup = async (tdGroupName: string): Promise<void> => {
  const oldTdGroups = await getTdGroups();
  const removeTargetIndex = oldTdGroups.findIndex((item) => {
    return item.name === tdGroupName;
  });

  if (removeTargetIndex === -1) {
    return Promise.reject(new Error(`Not found tdGroup ${tdGroupName}`));
  }

  oldTdGroups.splice(removeTargetIndex, 1);
  return fse.outputJSON(KF_TD_GROUP_JSON_PATH, oldTdGroups);
};

export const setTdGroup = (tdGroups: KungfuApi.KfExtraLocation[]) => {
  return fse.outputJSON(KF_TD_GROUP_JSON_PATH, tdGroups);
};

export const getScheduleTasks = async (): Promise<{
  active?: boolean;
  tasks?: KungfuApi.ScheduleTask[];
}> => {
  return fse.readFile(KF_SCHEDULE_TASKS_JSON_PATH).then((res) => {
    const str = Buffer.from(res).toString();
    return JSON.parse(str || '{}');
  });
};

export const setScheduleTasks = async (tasksConfig: {
  active: boolean;
  tasks: KungfuApi.ScheduleTask[];
}): Promise<void> => {
  return fse.outputJSON(KF_SCHEDULE_TASKS_JSON_PATH, tasksConfig);
};

export const getAllRiskSettingList = (): Promise<KungfuApi.RiskSetting[]> => {
  return getAllKfRiskSettings().then((riskSettingOrigins) => {
    const riskSettings = riskSettingOrigins
      .filter(
        (item) =>
          item.category === KfCategoryEnum.td && item.group && item.name,
      )
      .map((item) => {
        const riskSetting: KungfuApi.RiskSetting = JSON.parse(
          item.value,
        ) as KungfuApi.RiskSetting;
        return {
          ...riskSetting,
          account_id: getIdByKfLocation(item as KungfuApi.KfConfig),
        };
      });
    return Promise.resolve(riskSettings);
  });
};

export const setAllRiskSettingList = (
  riskSettings: KungfuApi.RiskSetting[],
) => {
  const riskSettingOrigins: KungfuApi.RiskSettingForSave[] = riskSettings
    .filter((item) => !!item.account_id)
    .map((item) => {
      const {
        account_id,
        max_order_volume,
        max_daily_volume,
        white_list,
        self_filled_check,
        max_cancel_ratio,
      } = item;

      const group = account_id.parseSourceAccountId().source;
      const name = account_id.parseSourceAccountId().id;

      return {
        location_uid: 0,
        category: 'td',
        group,
        name,
        mode: 'live',
        value: JSON.stringify({
          max_order_volume,
          max_daily_volume,
          white_list,
          self_filled_check,
          max_cancel_ratio,
        }),
      };
    });

  return setAllKfRiskSettings(riskSettingOrigins);
};
