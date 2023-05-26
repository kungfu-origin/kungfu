import {
  ComputedRef,
  Ref,
  ref,
  computed,
  watch,
  getCurrentInstance,
  toRaw,
  Component,
  App,
  h,
  InjectionKey,
  inject,
  provide,
} from 'vue';
import {
  ARCHIVE_DIR,
  buildProcessLogPath,
  KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  getInstrumentTypeData,
  getProcessIdByKfLocation,
  kfLogger,
  removeJournal,
  removeDB,
  getAvailDaemonList,
  getKfExtensionLanguage,
  loopToRunProcess,
  resolveInstrumentValue,
  transformSearchInstrumentResultToInstrument,
  removeArchiveBeforeToday,
  isKfColor,
  isHexOrRgbColor,
  removeTodayArchive,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { BrowserWindow, getCurrentWindow, dialog } from '@electron/remote';
import { ipcRenderer } from 'electron';
import { message, Modal, ModalFuncProps } from 'ant-design-vue';
import {
  InstrumentTypes,
  KfUIExtLocatorTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import path from 'path';
import { startExtDaemon } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { checkIfCpusNumSafe } from '@kungfu-trader/kungfu-js-api/utils/osUtils';
import { Proc } from 'pm2';
import { VueNode } from 'ant-design-vue/lib/_util/type';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;
import fse from 'fs-extra';
import md from 'markdown-it';
import { Router } from 'vue-router';

// this utils file is only for ui components

export const mergeExtLanguages = async () => {
  const languages = await getKfExtensionLanguage();
  console.log("Kungfu extension's languages:", languages);

  Object.keys(languages).forEach((langName) => {
    if (langName in VueI18n.global.messages) {
      VueI18n.global.mergeLocaleMessage(langName, languages[langName]);
    } else {
      console.warn(
        'Unregistered language: ' + langName,
        '\nLanguage data: ',
        languages[langName],
      );
    }
  });
};

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
  return Object.keys(kfUiExtConfigs)
    .filter((key) => {
      const config = kfUiExtConfigs[key];
      const { components, script } = config;
      return components || script;
    })
    .map((key) => {
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
            fse.pathExistsSync(path.join(extPath, (components || {})[cName])),
          )
          .reduce((cData, cName) => {
            return {
              ...cData,
              [`${key}-${cName}`]: globalThis.require(
                path.join(extPath, (components || {})[cName]),
              ).default as Component,
            };
          }, {} as Record<string, Component>),
      };
    });
};

export const loadExtScripts = async (
  components: {
    key: string;
    name: string;
    script: string;
    extPath: string;
    position: KfUIExtLocatorTypes;
    cData: Record<string, Component>;
  }[],
  app: App<Element>,
) => {
  const allExtScriptModules = await Promise.all(
    components.map(({ extPath, script }) => {
      const scriptPath = path.join(extPath, script);
      if (script && fse.pathExistsSync(scriptPath)) {
        return globalThis.require(scriptPath);
      }
    }),
  );

  allExtScriptModules
    .filter((extScriptModule) => !!extScriptModule)
    .forEach((extScriptModule) => {
      app.use(extScriptModule.default, globalThis);
    });

  return components;
};

export const loadExtComponents = (
  components: {
    key: string;
    name: string;
    script: string;
    extPath: string;
    position: KfUIExtLocatorTypes;
    cData: Record<string, Component>;
  }[],
  app: App<Element>,
  router: Router,
) => {
  components.forEach(({ cData, position, key, name }) => {
    switch (position) {
      case 'sidebar':
        if (cData[`${key}-entry`] && cData[`${key}-page`]) {
          app.component(key, cData[`${key}-entry`]);
          router.addRoute({
            path: `/${key}`,
            name: key,
            component: cData[`${key}-page`],
          });
        } else {
          console.warn(`${key}-entry or ${key}-page not in cData`);
        }
        break;
      case 'board':
        if (cData[`${key}-index`]) {
          app.component(name, cData[`${key}-index`]);
          if (app.config.globalProperties.$availKfBoards.indexOf(name) === -1) {
            app.config.globalProperties.$availKfBoards.push(name);
          }
        } else {
          console.warn(`${key}-index not in cData`);
        }
        break;
      default:
        if (cData[`${key}-index`]) {
          app.component(key, cData[`${key}-index`]);
        } else {
          console.warn(`${key}-index not in cData`);
        }
    }
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

export const useTreeTableSearchKeyword = <T extends { children?: T[] }>(
  targetList: Ref<T[]> | ComputedRef<T[]>,
  keys: string[],
): {
  searchKeyword: Ref<string>;
  tableData: Ref<T[]>;
} => {
  const searchKeyword = ref<string>('');
  function searchTree<T extends { children?: T[] }>(
    tree: T[],
    keys: string[],
    searchKeyword: string,
  ): T[] {
    return tree
      .filter((item) => {
        const combinedValue = keys
          .map((key: string) => {
            const keyWord = (item as Record<string, unknown>)[
              key
            ] as unknown as string | number;
            return keyWord ? keyWord.toString() : '';
          })
          .join('_');
        const isMatch = new RegExp(searchKeyword, 'ig').test(combinedValue);
        if (isMatch) return true;
        const childMatch =
          item.children && item.children.length > 0
            ? searchTree(item.children, keys, searchKeyword).length > 0
            : false;

        return childMatch;
      })
      .map((item) => ({
        ...item,
        children: searchTree(item.children || [], keys, searchKeyword),
      }));
  }

  const tableData = computed(() => {
    return searchTree<T>(targetList.value, keys, searchKeyword.value);
  });

  return {
    searchKeyword,
    tableData,
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
        return new RegExp(searchKeyword.value, 'ig').test(combinedValue);
      })
      .map((item) => toRaw(item));
  });

  return {
    searchKeyword,
    tableData,
  };
};

export const useWritableTableSearchKeyword = <T>(
  targetList: Ref<T[]> | ComputedRef<T[]>,
  keys: string[],
): {
  searchKeyword: Ref<string>;
  tableData: Ref<{ data: T; index: number; id: string }[]>;
} => {
  let id = 0;
  const idCachedMap = new WeakMap();
  const searchKeyword = ref<string>('');
  const tableData = ref<{ data: T; index: number; id: string }[]>([]) as Ref<
    { data: T; index: number; id: string }[]
  >;

  const generateItemId = (item: object) => {
    if (!idCachedMap.has(item)) idCachedMap.set(item, `${id++}`);
    return idCachedMap.get(item) as string;
  };

  watch(
    () => ({ keyword: searchKeyword.value, list: targetList.value }),
    (newValue) => {
      const { keyword, list } = newValue;
      tableData.value =
        list
          ?.map((item, index) => ({
            data: toRaw(item),
            index,
            id: generateItemId(item as unknown as object),
          }))
          .filter((item: { data: T; index: number }) => {
            const combinedValue = keys
              .map(
                (key: string) =>
                  (
                    ((item.data as Record<string, unknown>)[key] as
                      | string
                      | number) || ''
                  ).toString() || '',
              )
              .join('_');
            return new RegExp(keyword, 'ig').test(combinedValue);
          }) || [];
    },
    {
      deep: true,
      immediate: true,
    },
  );

  return {
    searchKeyword,
    tableData,
  };
};

const removeArchiveBeforeStartAll = (): Promise<void> => {
  return removeArchiveBeforeToday(ARCHIVE_DIR).then(() => {
    kfLogger.info('Clear Archive Done');
  });
};

const removeJournalBeforeStartAll = (): Promise<void> => {
  const needClearJournalStr = localStorage.getItem('needClearJournal');
  const needClearJournal = !!(needClearJournalStr && +needClearJournalStr);

  kfLogger.info('needClearJournal: ', needClearJournal);

  if (needClearJournal) {
    localStorage.setItem('needClearJournal', '0');
    kfLogger.info('Clear Journal Done', needClearJournal);
    return removeTodayArchive(ARCHIVE_DIR).then(() => removeJournal(KF_HOME));
  } else {
    return Promise.resolve();
  }
};

const removeDBBeforeStartAll = (): Promise<void> => {
  const needClearDBStr = localStorage.getItem('needClearDB');
  const needClearDB = !!(needClearDBStr && +needClearDBStr);

  kfLogger.info('needClearDB: ', needClearDB);

  if (needClearDB) {
    localStorage.setItem('needClearDB', '0');
    kfLogger.info('Clear DB Done');
    return removeDB(KF_HOME);
  } else {
    return Promise.resolve();
  }
};

export const preStartAll = async (): Promise<(void | Proc)[]> => {
  return Promise.all([
    removeJournalBeforeStartAll(),
    removeDBBeforeStartAll(),
    removeArchiveBeforeStartAll(),
  ]);
};

export const checkCpusNumAndConfirmModal = (): Promise<boolean> => {
  return checkIfCpusNumSafe().then((flag) => {
    if (flag) return Promise.resolve(true);

    return confirmModalByCustomArgs(
      t('system_prompt'),
      t('computer_performance_abnormal'),
      { zIndex: 1001 },
    );
  });
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
      },
      backgroundColor: '#000',
      ...windowConfig,
    });

    win.on('ready-to-show', function () {
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

export const setHtmlTitle = (extraTitle?: string): void => {
  const rootPackageJson = readRootPackageJsonSync();
  const titleResolved = rootPackageJson?.kungfuCraft?.appTitle || t('kungfu');
  document.getElementsByTagName('title')[0].innerText = [
    `${titleResolved}`,
    extraTitle,
  ]
    .filter((title) => !!title)
    .join(' - ');
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
  ipcRenderer.on('main-process-messages', (_event, name, payload) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'main',
        name,
        payload,
      } as KfEvent.MainProcessEvent);
    }
  });
};

export const markClearJournal = (): void => {
  localStorage.setItem('needClearJournal', '1');
  messagePrompt().success(t('clear', { content: 'journal' }));
};

export const markClearDB = (): void => {
  localStorage.setItem('needClearDB', '1');
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
    message.error(msg, 5);
  };
  const warning = (msg: string): void => {
    message.warning(msg, 5);
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
  const logPath = buildProcessLogPath(getProcessIdByKfLocation(config));
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
      } as KfEvent.ExportTradingDataEvent);
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
    ExchangeIds[instrument.exchangeId.toUpperCase()]?.name || ''
  }`;
};

export const makeSearchOptionFormInstruments = (
  type: 'instrument' | 'instruments' | 'instrumentsCsv',
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
): Promise<boolean> => {
  return new Promise((resolve) => {
    Modal.confirm({
      title: title,
      content: content,
      okText: okText,
      cancelText: cancelText,
      onOk: () => {
        resolve(true);
      },
      onCancel: () => {
        resolve(false);
      },
    });
  });
};

export const confirmModalByCustomArgs = (
  title: string,
  content: VueNode | (() => VueNode) | string,
  args: ModalFuncProps = {},
): Promise<boolean> => {
  return new Promise((resolve) => {
    Modal.confirm({
      title,
      content,
      ...args,
      okText: args?.okText || t('confirm'),
      cancelText: args?.cancelText || t('cancel'),
      zIndex: args?.zIndex || 1000,
      onOk: () => {
        resolve(true);
      },
      onCancel: () => {
        resolve(false);
      },
    });
  });
};

const markdown = md();

export const openReadmeModal = (title: string, readmePath: string) => {
  if (fse.existsSync(readmePath)) {
    return fse.readFile(readmePath).then((buffer) => {
      const str = buffer.toString();
      const mdHtml = markdown.render(str);
      const content = h('div', {
        class: 'kf-modal-markdown__wrap',
        innerHTML: mdHtml,
      });
      return Modal.confirm({
        title: title,
        content: content,
        width: 600,
        okText: t('confirm'),
        cancelText: t('cancel'),
      });
    });
  } else {
    message.error(t('文件路径不存在'));
    return Promise.reject();
  }
};

export const useBoardFilter = () => {
  const rootPackageJson = readRootPackageJsonSync();
  const boardFilter: Record<string, boolean | undefined> | undefined =
    rootPackageJson?.boardFilter;

  const getBoard = <T>(boardName: string, ifTrue: T, ifFalse: T): T => {
    return boardFilter ? (boardFilter[boardName] ? ifTrue : ifFalse) : ifTrue;
  };

  return {
    boardFilter,
    getBoard,
  };
};

export const dealKungfuColorToClassname = (
  color: KungfuApi.AntInKungfuColorTypes,
) => {
  return isKfColor(color)
    ? color
    : !isHexOrRgbColor(color)
    ? `color-${color || 'default'}`
    : '';
};

export const dealKungfuColorToStyleColor = (
  color: KungfuApi.AntInKungfuColorTypes,
) => {
  return isKfColor(color) ? '' : color;
};

// eslint-disable-next-line @typescript-eslint/no-explicit-any
export const vueProvideBaseOnParent = <T extends { [x: string]: any }>(
  key: InjectionKey<T> | string,
  value: T,
) => {
  const parentProvide = inject(key);
  if (!parentProvide) return provide(key, value);
  if (typeof parentProvide !== 'object' || typeof value !== 'object')
    return provide(key, value);
  return provide(key, Object.assign(parentProvide, value));
};
