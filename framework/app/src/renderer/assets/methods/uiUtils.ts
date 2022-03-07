import {
  ComputedRef,
  Ref,
  reactive,
  ref,
  computed,
  getCurrentInstance,
  onMounted,
  toRefs,
  toRaw,
  Component,
} from 'vue';
import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  buildExtTypeMap,
  dealCategory,
  getAppStateStatusName,
  getIdByKfLocation,
  getInstrumentTypeData,
  getProcessIdByKfLocation,
  getTradingDate,
  kfLogger,
  removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

import {
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { storeToRefs } from 'pinia';
import { BrowserWindow, getCurrentWindow } from '@electron/remote';
import { ipcRenderer } from 'electron';
import { message } from 'ant-design-vue';
import {
  InstrumentTypes,
  BrokerStateStatusTypes,
  ProcessStatusTypes,
  KfCategoryTypes,
  KfUIExtLocatorTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import dayjs from 'dayjs';
import path from 'path';

// this utils file is only for ui components
export const getUIComponents = (
  kfUiExtConfigs: KungfuApi.KfUIExtConfigs,
): {
  key: string;
  name: string;
  position: KfUIExtLocatorTypes;
  cData: Record<string, Component>;
}[] => {
  return Object.keys(kfUiExtConfigs).map((key) => {
    const config = kfUiExtConfigs[key];
    const { extPath, position, components, name } = config;
    return {
      key,
      name,
      position,
      cData: Object.keys(components || {}).reduce((cData, cName) => {
        return {
          ...cData,
          [`${key}-${cName}`]: global.require(
            path.join(extPath, components[cName]),
          ).default as Component,
        };
      }, {} as Record<string, Component>),
    };
  });
};

export const useModalVisible = (
  visible: boolean,
): { modalVisible: Ref<boolean>; closeModal: () => void } => {
  const app = getCurrentInstance();
  const modalVisible = ref<boolean>(visible);

  const closeModal = () => {
    app && app.emit('update:visible', false);
    app && app.emit('close');
  };

  return {
    modalVisible,
    closeModal,
  };
};

export const useTableSearchKeyword = <T>(
  targetList: Ref<T[]> | ComputedRef<T[]>,
  keys: string[],
): {
  searchKeyword: Ref<string>;
  tableData: ComputedRef<T[]>;
} => {
  const searchKeyword = ref<string>('');
  const tableData = computed(() => {
    return targetList.value
      .filter((item: T) => {
        const combinedValue = keys
          .map(
            (key: string) =>
              (
                ((item as Record<string, unknown>)[key] as string | number) ||
                ''
              ).toString() || '',
          )
          .join('_');
        return combinedValue.includes(searchKeyword.value);
      })
      .map((item) => toRaw(item));
  });

  return {
    searchKeyword,
    tableData,
  };
};

export const beforeStartAll = (): Promise<void> => {
  const clearJournalDateFromLocal = localStorage.getItem(
    'clearJournalTradingDate',
  );
  const currentTradingDate = getTradingDate();
  kfLogger.info(
    'Lastest Clear Journal Trading Date: ',
    clearJournalDateFromLocal || '',
  );

  if (currentTradingDate !== clearJournalDateFromLocal) {
    localStorage.setItem('clearJournalTradingDate', currentTradingDate);
    kfLogger.info('Clear Journal Trading Date: ', currentTradingDate);
    return removeJournal(KF_HOME);
  } else {
    return Promise.resolve();
  }
};

export const getInstrumentTypeColor = (
  type: InstrumentTypes,
): KungfuApi.AntInKungfuColorTypes => {
  return getInstrumentTypeData(type).color || 'default';
};

export const useExtConfigsRelated = (): {
  extConfigs: { data: KungfuApi.KfExtConfigs };
  uiExtConfigs: { data: KungfuApi.KfUIExtConfigs };
  tdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
  mdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
} => {
  const app = getCurrentInstance();
  const extConfigs = reactive<{ data: KungfuApi.KfExtConfigs }>({
    data: {},
  });
  const uiExtConfigs = reactive<{ data: KungfuApi.KfUIExtConfigs }>({
    data: {},
  });
  const tdExtTypeMap = computed(() => buildExtTypeMap(extConfigs.data, 'td'));
  const mdExtTypeMap = computed(() => buildExtTypeMap(extConfigs.data, 'md'));

  onMounted(() => {
    if (app?.proxy) {
      const store = storeToRefs(app?.proxy.$useGlobalStore());
      extConfigs.data = store.extConfigs as unknown as KungfuApi.KfExtConfigs;
      uiExtConfigs.data =
        store.uiExtConfigs as unknown as KungfuApi.KfUIExtConfigs;
    }
  });

  return {
    extConfigs,
    uiExtConfigs,
    tdExtTypeMap,
    mdExtTypeMap,
  };
};

export const useProcessStatusDetailData = (): {
  processStatusData: Ref<Pm2ProcessStatusData>;
  processStatusDetailData: Ref<Pm2ProcessStatusDetailData>;
  appStates: Ref<Record<string, BrokerStateStatusTypes>>;
  getProcessStatusName(
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): ProcessStatusTypes | undefined;
} => {
  const app = getCurrentInstance();
  const allProcessStatusData = reactive<{
    processStatusData: Pm2ProcessStatusData;
    processStatusDetailData: Pm2ProcessStatusDetailData;
    appStates: Record<string, BrokerStateStatusTypes>;
  }>({
    processStatusData: {},
    processStatusDetailData: {},
    appStates: {},
  });

  onMounted(() => {
    if (app?.proxy) {
      const { processStatusData, processStatusWithDetail, appStates } =
        storeToRefs(app?.proxy.$useGlobalStore());
      allProcessStatusData.processStatusData =
        processStatusData as unknown as Pm2ProcessStatusData;
      allProcessStatusData.processStatusDetailData =
        processStatusWithDetail as unknown as Pm2ProcessStatusDetailData;
      allProcessStatusData.appStates = appStates as unknown as Record<
        string,
        BrokerStateStatusTypes
      >;
    }
  });

  const getProcessStatusName = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ) => {
    return getAppStateStatusName(
      kfConfig,
      allProcessStatusData.processStatusData,
      allProcessStatusData.appStates,
    );
  };

  const { processStatusData, processStatusDetailData, appStates } =
    toRefs(allProcessStatusData);

  return {
    processStatusData,
    processStatusDetailData,
    appStates,
    getProcessStatusName,
  };
};

export const useAllKfConfigData = (): Record<
  KfCategoryTypes,
  KungfuApi.KfConfig[]
> => {
  const app = getCurrentInstance();
  const allKfConfigData: Record<KfCategoryTypes, KungfuApi.KfConfig[]> =
    reactive({
      system: ref<KungfuApi.KfConfig[]>([
        ...(process.env.NODE_ENV === 'development'
          ? ([
              {
                location_uid: 0,
                category: 'system',
                group: 'service',
                name: 'archive',
                mode: 'live',
                value: '',
              },
            ] as KungfuApi.KfConfig[])
          : []),
        {
          location_uid: 0,
          category: 'system',
          group: 'master',
          name: 'master',
          mode: 'live',
          value: '',
        },
        {
          location_uid: 0,
          category: 'system',
          group: 'service',
          name: 'ledger',
          mode: 'live',
          value: '',
        },
      ]),

      md: [],
      td: [],
      strategy: [],
    });

  onMounted(() => {
    if (app?.proxy) {
      const { mdList, tdList, strategyList } = storeToRefs(
        app?.proxy.$useGlobalStore(),
      );

      allKfConfigData.md = mdList as unknown as KungfuApi.KfConfig[];
      allKfConfigData.td = tdList as unknown as KungfuApi.KfConfig[];
      allKfConfigData.strategy =
        strategyList as unknown as KungfuApi.KfConfig[];
    }
  });

  return allKfConfigData;
};

export const useTdGroups = (): { data: KungfuApi.KfExtraLocation[] } => {
  const app = getCurrentInstance();
  const tdGroups = reactive<{ data: KungfuApi.KfExtraLocation[] }>({
    data: [],
  });

  onMounted(() => {
    if (app?.proxy) {
      const { tdGroupList } = storeToRefs(app?.proxy.$useGlobalStore());
      tdGroups.data = tdGroupList as unknown as KungfuApi.KfExtraLocation[];
    }
  });

  return tdGroups;
};

export const useCurrentGlobalKfLocation = (
  watcher: KungfuApi.Watcher | null,
): {
  currentGlobalKfLocation: {
    data: KungfuApi.KfLocation | KungfuApi.KfConfig | null;
  };
  currentCategoryData: ComputedRef<KungfuApi.KfTradeValueCommonData | null>;
  currentUID: ComputedRef<string>;
  setCurrentGlobalKfLocation(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): void;
  dealRowClassName(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): string;
  customRow(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): {
    onClick(): void;
  };
  getCurrentGlobalKfLocationId(
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ): string;
} => {
  const app = getCurrentInstance();
  const currentKfLocation = reactive<{
    data: KungfuApi.KfLocation | KungfuApi.KfConfig | null;
  }>({
    data: null,
  });

  onMounted(() => {
    if (app?.proxy) {
      const { currentGlobalKfLocation } = storeToRefs(
        app?.proxy.$useGlobalStore(),
      );

      currentKfLocation.data = currentGlobalKfLocation as unknown as
        | KungfuApi.KfLocation
        | KungfuApi.KfConfig
        | null;
    }
  });

  const setCurrentGlobalKfLocation = (
    kfLocation:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ) => {
    if (app?.proxy) {
      app?.proxy?.$useGlobalStore().setCurrentGlobalKfLocation(kfLocation);
    }
  };

  const dealRowClassName = (
    record:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): string => {
    if (currentKfLocation.data === null) return '';

    if (
      getIdByKfLocation(record) === getIdByKfLocation(currentKfLocation.data)
    ) {
      return 'current-global-kfLocation';
    }

    return '';
  };

  const customRow = (
    record:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ) => {
    return {
      onClick: () => {
        setCurrentGlobalKfLocation(record);
      },
    };
  };

  const currentCategoryData = computed(() => {
    if (!currentKfLocation.data) {
      return null;
    }

    const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
      app?.proxy ? app?.proxy.$globalCategoryRegister.getExtraCategory() : {};

    return dealCategory(currentKfLocation.data.category, extraCategory);
  });

  const currentUID = computed(() => {
    if (!watcher) {
      return '';
    }

    if (!currentKfLocation.data) {
      return '';
    }

    return watcher.getLocationUID(currentKfLocation.data);
  });

  const getCurrentGlobalKfLocationId = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ): string => {
    if (!kfConfig) {
      return '';
    }

    return getIdByKfLocation(kfConfig) || '';
  };

  return {
    currentGlobalKfLocation: currentKfLocation,
    currentCategoryData,
    currentUID,
    setCurrentGlobalKfLocation,
    dealRowClassName,
    customRow,
    getCurrentGlobalKfLocationId,
  };
};

/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openNewBrowserWindow = (
  name: string,
  params: string,
  windowConfig?: Electron.BrowserWindowConstructorOptions,
): Promise<Electron.BrowserWindow> => {
  const currentWindow = getCurrentWindow();

  const modalPath =
    process.env.NODE_ENV !== 'production'
      ? `http://localhost:9090/${name}.html${params}`
      : `file://${__dirname}/${name}.html${params}`;

  return new Promise((resolve, reject) => {
    const win = new BrowserWindow({
      ...(getNewWindowLocation() || {}),
      width: 1080,
      height: 766,
      parent: currentWindow,
      webPreferences: {
        nodeIntegration: true,
        nodeIntegrationInWorker: true,
        contextIsolation: false,
        enableRemoteModule: true,
      },
      backgroundColor: '#000',
      ...windowConfig,
    });

    win.on('ready-to-show', function () {
      win && win.show();
      win && win.focus();
    });

    win.webContents.loadURL(modalPath);
    win.webContents.on('did-finish-load', () => {
      if (!currentWindow || Object.keys(currentWindow).length == 0) {
        reject(new Error('当前页面没有聚焦'));
        return;
      }
      resolve(win);
    });
  });
};

function getNewWindowLocation(): { x: number; y: number } | null {
  const currentWindow = getCurrentWindow();
  if (currentWindow) {
    //如果上一步中有活动窗口, 则根据当前活动窗口的右下方设置下一个窗口的坐标
    const [currentWindowX, currentWindowY] = currentWindow.getPosition();
    const x = currentWindowX + 10;
    const y = currentWindowY + 10;

    return {
      x,
      y,
    };
  }

  return null;
}

export const openLogView = (
  processId: string,
): Promise<Electron.BrowserWindow> => {
  return openNewBrowserWindow('logview', `?processId=${processId}`);
};

export const removeLoadingMask = (): void => {
  const $loadingMask = document.getElementById('loading');
  if ($loadingMask) $loadingMask.remove();
};

export const setHtmlTitle = (title: string): void => {
  document.getElementsByTagName('title')[0].innerText = title;
};

export const parseURIParams = (): Record<string, string> => {
  const search = window.location.search;
  const searchResolved = search.slice(search.indexOf('?') + 1);
  const searchResolvedSplits = searchResolved.split('&');
  const paramsData: Record<string, string> = {};
  searchResolvedSplits.forEach((item: string) => {
    const itemSplit = item.split('=');
    if (itemSplit.length === 2) {
      paramsData[itemSplit[0] || ''] = itemSplit[1] || '';
    }
  });

  return paramsData;
};

export const useIpcListener = (): void => {
  const app = getCurrentInstance();
  ipcRenderer.removeAllListeners('main-process-messages');
  ipcRenderer.on('main-process-messages', (event, args) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'main',
        name: args,
      } as MainProcessEvent);
    }
  });
};

export const markClearJournal = (): void => {
  localStorage.setItem('clearJournalTradingDate', '');
  message.success('清理 journal 完成, 请重启应用');
};

export const handleOpenLogview = (
  config: KungfuApi.KfConfig | KungfuApi.KfLocation,
): Promise<Electron.BrowserWindow | void> => {
  const hideloading = message.loading('正在打开窗口');
  return openLogView(getProcessIdByKfLocation(config)).finally(() => {
    hideloading();
  });
};

export const useDashboardBodySize = (): {
  dashboardBodyHeight: Ref;
  dashboardBodyWidth: Ref;
  handleBodySizeChange({
    width,
    height,
  }: {
    width: number;
    height: number;
  }): void;
} => {
  const dashboardBodyHeight = ref<number>(0);
  const dashboardBodyWidth = ref<number>(0);
  const handleBodySizeChange = ({
    width,
    height,
  }: {
    width: number;
    height: number;
  }) => {
    const tableHeaderHeight = 36;
    dashboardBodyHeight.value = height - tableHeaderHeight;
    dashboardBodyWidth.value = width > 800 ? 800 : width;
  };

  return {
    dashboardBodyHeight,
    dashboardBodyWidth,
    handleBodySizeChange,
  };
};

export const useAssets = (): {
  assets: { data: Record<string, KungfuApi.Asset> };
  getAssetsByKfConfig(
    kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): KungfuApi.Asset;
  getAssetsByTdGroup(tdGroup: KungfuApi.KfExtraLocation): KungfuApi.Asset;
} => {
  const assetsResolved = reactive<{ data: Record<string, KungfuApi.Asset> }>({
    data: {},
  });

  const app = getCurrentInstance();

  onMounted(() => {
    if (app?.proxy) {
      const { assets } = storeToRefs(app?.proxy.$useGlobalStore());
      assetsResolved.data = assets as unknown as Record<
        string,
        KungfuApi.Asset
      >;
    }
  });

  const getAssetsByKfConfig = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): KungfuApi.Asset => {
    const processId = getProcessIdByKfLocation(kfConfig);
    return assetsResolved.data[processId] || ({} as KungfuApi.Asset);
  };

  const getAssetsByTdGroup = (
    tdGroup: KungfuApi.KfExtraLocation,
  ): KungfuApi.Asset => {
    const children = (tdGroup.children || []) as KungfuApi.KfConfig[];
    const assetsList = children
      .map((item) => getAssetsByKfConfig(item))
      .filter((item) => Object.keys(item).length);

    return assetsList.reduce((allAssets, asset) => {
      return {
        ...allAssets,
        unrealized_pnl: (allAssets.unrealized_pnl || 0) + asset.unrealized_pnl,
        market_value: (allAssets.market_value || 0) + asset.market_value,
        margin: (allAssets.margin || 0) + asset.margin,
        avail: (allAssets.avail || 0) + asset.avail,
      };
    }, {} as KungfuApi.Asset);
  };

  return {
    assets: assetsResolved,
    getAssetsByKfConfig,
    getAssetsByTdGroup,
  };
};

export const getKfLocationUID = (kfConfig: KungfuApi.KfConfig): string => {
  if (!window.watcher) return '';
  return window.watcher?.getLocationUID(kfConfig);
};

export const useDownloadHistoryTradingData = (): {
  handleDownload: (
    tradingDataType: KungfuApi.TradingDataTypeName | 'all',
    currentKfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ) => void;
} => {
  const app = getCurrentInstance();

  const handleDownload = (
    tradingDataType: KungfuApi.TradingDataTypeName | 'all',
    currentKfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ): void => {
    if (!currentKfLocation) {
      return;
    }

    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'export',
        tradingDataType,
        currentKfLocation,
      } as ExportTradingDataEvent);
    }
  };

  return {
    handleDownload,
  };
};

export const buildInstrumentSelectOptionValue = (
  instrument: KungfuApi.InstrumentResolved,
): string => {
  return `${instrument.exchangeId}_${instrument.instrumentId}_${instrument.instrumentType}_${instrument.ukey}_${instrument.instrumentName}`;
};

export const buildInstrumentSelectOptionLabel = (
  instrument: KungfuApi.InstrumentResolved,
): string => {
  return `${instrument.instrumentId} ${instrument.instrumentName} ${
    ExchangeIds[instrument.exchangeId.toUpperCase()].name
  }`;
};

export const useTriggerMakeOrder = (): {
  customRow(
    instrument: KungfuApi.InstrumentResolved,
    callback: (instrument: KungfuApi.InstrumentResolved) => void,
  ): { onClick(): void };
  triggerOrderBook(instrument: KungfuApi.InstrumentResolved): void;
  triggerOrderBookUpdate(
    instrument: KungfuApi.InstrumentResolved,
    extraOrderInput: ExtraOrderInput,
  ): void;
  triggerMakeOrder(
    instrument: KungfuApi.InstrumentResolved,
    extraOrderInput: ExtraOrderInput,
  ): void;
} => {
  const app = getCurrentInstance();

  const triggerOrderBook = (instrument: KungfuApi.InstrumentResolved) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'orderbook',
        instrument,
      });
    }
  };

  const triggerOrderBookUpdate = (
    instrument: KungfuApi.InstrumentResolved,
    extraOrderInput: ExtraOrderInput,
  ) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'orderBookUpdate',
        orderInput: {
          ...instrument,
          ...(extraOrderInput || {}),
        },
      });
    }
  };

  const triggerMakeOrder = (
    instrument: KungfuApi.InstrumentResolved,
    extraOrderInput: ExtraOrderInput,
  ) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'makeOrder',
        orderInput: {
          ...instrument,
          ...(extraOrderInput || {}),
        },
      });
    }
  };

  const customRow = (
    record: KungfuApi.InstrumentResolved,
    callback: (instrument: KungfuApi.InstrumentResolved) => void,
  ) => {
    return {
      onClick: () => {
        callback(record);
      },
    };
  };

  return {
    customRow,
    triggerOrderBook,
    triggerOrderBookUpdate,
    triggerMakeOrder,
  };
};

export const isInTdGroup = (
  tdGroup: KungfuApi.KfExtraLocation[],
  accountId: string,
): KungfuApi.KfExtraLocation | null => {
  const targetGroups = tdGroup.filter((item) => {
    return item.children?.includes(accountId);
  });
  return targetGroups[0] || null;
};
