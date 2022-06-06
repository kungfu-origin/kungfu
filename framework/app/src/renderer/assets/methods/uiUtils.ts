import {
  ComputedRef,
  Ref,
  ref,
  computed,
  getCurrentInstance,
  toRaw,
  Component,
} from 'vue';
import {
  KF_HOME,
  LOG_DIR,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  getInstrumentTypeData,
  getProcessIdByKfLocation,
  getTradingDate,
  kfLogger,
  removeJournal,
  removeDB,
  getAvailDaemonList,
  loopToRunProcess,
  resolveInstrumentValue,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import dayjs from 'dayjs';
import { BrowserWindow, getCurrentWindow, dialog } from '@electron/remote';
import { ipcRenderer } from 'electron';
import { message, Modal } from 'ant-design-vue';
import {
  InstrumentTypes,
  KfUIExtLocatorTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import path from 'path';
import { startExtDaemon } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { Proc } from 'pm2';
import { VueNode } from 'ant-design-vue/lib/_util/type';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;
import fse from 'fs-extra';

// this utils file is only for ui components
export const getUIComponents = (
  kfUiExtConfigs: KungfuApi.KfUIExtConfigs,
): {
  key: string;
  name: string;
  script: string;
  extPath: string;
  position: KfUIExtLocatorTypes;
  cData: Record<string, Component>;
}[] => {
  return Object.keys(kfUiExtConfigs).map((key) => {
    const config = kfUiExtConfigs[key];
    const { extPath, position, components, name, script } = config;

    return {
      key,
      name,
      position,
      script,
      extPath,
      cData: Object.keys(components || {})
        .filter((cName) =>
          fse.pathExistsSync(path.join(extPath, components[cName])),
        )
        .reduce((cData, cName) => {
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

const removeJournalBeforeStartAll = (
  currentTradingDate: string,
): Promise<void> => {
  const clearJournalDateFromLocal = localStorage.getItem(
    'clearJournalTradingDate',
  );

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

const removeDBBeforeStartAll = (currentTradingDate: string): Promise<void> => {
  const clearDBDateFromLocal = localStorage.getItem('clearDBTradingDate');

  kfLogger.info('Lastest Clear DB Trading Date: ', clearDBDateFromLocal || '');

  if (currentTradingDate !== clearDBDateFromLocal) {
    localStorage.setItem('clearDBTradingDate', currentTradingDate);
    kfLogger.info('Clear DB Trading Date: ', currentTradingDate);
    return removeDB(KF_HOME);
  } else {
    return Promise.resolve();
  }
};

export const preStartAll = async (): Promise<(void | Proc)[]> => {
  const currentTradingDate = getTradingDate();
  return Promise.all([
    removeJournalBeforeStartAll(currentTradingDate),
    removeDBBeforeStartAll(currentTradingDate),
  ]);
};

export const postStartAll = async (): Promise<(void | Proc)[]> => {
  const availDaemons = await getAvailDaemonList();
  return loopToRunProcess<void | Proc>(
    availDaemons.map((item) => {
      return () =>
        startExtDaemon(getProcessIdByKfLocation(item), item.cwd, item.script)
          .then((res) => {
            return res;
          })
          .catch((err) => console.error(err));
    }),
  );
};

export const getInstrumentTypeColor = (
  type: InstrumentTypes,
): KungfuApi.AntInKungfuColorTypes => {
  return getInstrumentTypeData(type).color || 'default';
};

/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openNewBrowserWindow = (
  folderName: string,
  name: string,
  params = '',
  windowConfig?: Electron.BrowserWindowConstructorOptions,
): Promise<Electron.BrowserWindow> => {
  const currentWindow = getCurrentWindow();
  const modalPath =
    process.env.APP_TYPE === 'renderer' && process.env.NODE_ENV !== 'production'
      ? `http://localhost:9090/${name}.html${params}`
      : `file://${folderName}/${name}.html${params}`;

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
        reject(new Error(t('no_focus')));
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
  logPath: string,
): Promise<Electron.BrowserWindow> => {
  return openNewBrowserWindow(__dirname, 'logview', `?logPath=${logPath}`);
};

export const openCodeView = (
  processId: string,
): Promise<Electron.BrowserWindow> => {
  return openNewBrowserWindow(__dirname, 'code', `?processId=${processId}`);
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
  ipcRenderer.on('main-process-messages', (_event, args) => {
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
  messagePrompt().success(t('clear', { content: 'journal' }));
};

export const markClearDB = (): void => {
  localStorage.setItem('clearDBTradingDate', '');
  messagePrompt().success(t('clear', { content: 'DB' }));
};

export const messagePrompt = (): {
  success(msg?: string): void;
  error(msg?: string): void;
  warning(msg: string): void;
} => {
  const success = (msg: string = t('operation_success')): void => {
    message.success(msg);
  };
  const error = (msg: string = t('operation_failed')): void => {
    message.error(msg);
  };
  const warning = (msg: string): void => {
    message.warning(msg);
  };
  return {
    success,
    error,
    warning,
  };
};

export const handleOpenLogview = (
  config: KungfuApi.KfConfig | KungfuApi.KfLocation,
): Promise<Electron.BrowserWindow | void> => {
  const hideloading = message.loading(t('open_window'));
  const logPath = path.resolve(
    LOG_DIR,
    dayjs().format('YYYYMMDD'),
    `${getProcessIdByKfLocation(config)}.log`,
  );
  return openLogView(logPath).finally(() => {
    hideloading();
  });
};

export const handleOpenLogviewByFile =
  (): Promise<Electron.BrowserWindow | void> => {
    return dialog
      .showOpenDialog({
        properties: ['openFile'],
      })
      .then((res): Promise<Electron.BrowserWindow | void> => {
        const { filePaths } = res;
        if (filePaths.length) {
          const targetLogPath = filePaths[0];
          const hideloading = message.loading(t('open_window'));
          return openLogView(targetLogPath).finally(() => {
            hideloading();
          });
        }

        return Promise.resolve();
      });
  };

export const handleOpenCodeView = (
  config: KungfuApi.KfConfig | KungfuApi.KfLocation,
): Promise<Electron.BrowserWindow> => {
  const openMessage = message.loading(t('open_code_editor'));
  return openCodeView(getProcessIdByKfLocation(config)).finally(() => {
    openMessage();
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

export const makeSearchOptionFormInstruments = (
  type: 'instrument' | 'instruments',
  value: string | string[],
): { value: string; label: string }[] => {
  const valResolved = resolveInstrumentValue(type, value);
  const instrumentResolveds: Array<KungfuApi.InstrumentResolved> = valResolved
    .map((item) => {
      return transformSearchInstrumentResultToInstrument(item.toString());
    })
    .filter((item): item is KungfuApi.InstrumentResolved => !!item);

  return [
    ...instrumentResolveds.map((item) => ({
      value: buildInstrumentSelectOptionValue(item),
      label: buildInstrumentSelectOptionLabel(item),
    })),
  ];
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

export const confirmModal = (
  title: string,
  content: VueNode | (() => VueNode) | string,
  okText = t('confirm'),
  cancelText = t('cancel'),
): Promise<void> => {
  return new Promise((resolve) => {
    Modal.confirm({
      title: title,
      content: content,
      okText: okText,
      cancelText: cancelText,
      onOk: () => {
        resolve();
      },
    });
  });
};
