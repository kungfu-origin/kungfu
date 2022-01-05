import fs from 'fs-extra';
import path from 'path';
import {
    Component,
    ComputedRef,
    Ref,
    reactive,
    ref,
    computed,
    getCurrentInstance,
    onMounted,
    toRefs,
    toRaw,
} from 'vue';
import {
    APP_DIR,
    KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
    buildExtTypeMap,
    buildObjectFromArray,
    dealCategory,
    getAppStateStatusName,
    getIdByKfLocation,
    getInstrumentTypeData,
    getProcessIdByKfLocation,
    getTradingDate,
    kfLogger,
    removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    CommissionMode,
    Direction,
    ExchangeIds,
    HedgeFlag,
    InstrumentType,
    Offset,
    PriceType,
    Side,
    TimeCondition,
    VolumeCondition,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

import {
    Pm2ProcessStatusData,
    Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { storeToRefs } from 'pinia';
import { BrowserWindow, getCurrentWindow } from '@electron/remote';
import { ipcRenderer } from 'electron';
import { message } from 'ant-design-vue';
import dayjs from 'dayjs';
import {
    InstrumentTypes,
    BrokerStateStatusTypes,
    KfCategoryTypes,
    ProcessStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

export interface KfUIComponent {
    name: string;
    component: Component;
}

// this utils file is only for ui components
export const getUIComponents = (): {
    [prop: string]: KfUIComponent[keyof KfUIComponent] | null | undefined;
    [prop: number]: KfUIComponent[keyof KfUIComponent] | null | undefined;
} => {
    const componentsDir = path.join(APP_DIR, 'components');
    const files = fs.readdirSync(componentsDir);
    const jsFiles = files.filter((file) => file.includes('.js'));
    const existedNames: string[] = [];
    const uicList = jsFiles
        .map((file: string): KfUIComponent | null => {
            const fullpath = path.join(componentsDir, file);
            const uic = global.require(fullpath).default as Component;

            if (!uic) {
                return null;
            }

            const { name } = uic;
            if (!name) {
                console.error('no name property in components' + fullpath);
                return null;
            }

            if (existedNames.includes(name)) {
                console.error(`component name ${name} is existed, ${fullpath}`);
            }

            return {
                name,
                component: uic,
            };
        })
        .filter((item: KfUIComponent | null) => !!item);

    return buildObjectFromArray<KfUIComponent | null>(
        uicList,
        'name',
        'component',
    );
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
        return targetList.value.filter((item: T) => {
            const combinedValue = keys
                .map(
                    (key: string) =>
                        (
                            ((item as Record<string, unknown>)[key] as
                                | string
                                | number) || ''
                        ).toString() || '',
                )
                .join('_');
            return combinedValue.includes(searchKeyword.value);
        });
    });

    return {
        searchKeyword,
        tableData,
    };
};

export const initFormStateByConfig = (
    configSettings: KungfuApi.KfConfigItem[],
    initValue?: Record<string, KungfuApi.KfConfigValue>,
): Record<string, KungfuApi.KfConfigValue> => {
    if (!configSettings) return {};

    const booleanType = ['bool'];
    const numberType = [
        'int',
        'float',
        'percent',
        'side', // select - number
        'offset', // select - number
        'direction', // select - number
        'priceType', // select - number
        'hedgeFlag', // select - number
        'volumeCondition', // select - number
        'timeCondition', // select - number
        'commissionMode', // select - number
        'instrumentType', // select - number
    ];
    const formState: Record<string, KungfuApi.KfConfigValue> = {};
    configSettings.forEach((item) => {
        const type = item.type;
        const isBoolean = booleanType.includes(type);
        const isNumber = numberType.includes(type);

        let defaultValue;
        if (typeof item?.default === 'object') {
            defaultValue = JSON.parse(JSON.stringify(item?.default));
        } else {
            defaultValue = item?.default;
        }

        if (defaultValue === undefined) {
            defaultValue = isBoolean ? false : isNumber ? 0 : '';
        }
        if ((initValue || {})[item.key] !== undefined) {
            defaultValue = (initValue || {})[item.key];
        }

        formState[item.key] = defaultValue;
    });

    return formState;
};

export const numberEnumRadioType: Record<
    string,
    Record<number, KungfuApi.KfTradeValueCommonData>
> = {
    offset: Offset,
    hedgeFlag: HedgeFlag,
    direction: Direction,
    volumeCondition: VolumeCondition,
    timeCondition: TimeCondition,
    commissionMode: CommissionMode,
};

export const numberEnumSelectType: Record<
    string,
    Record<number, KungfuApi.KfTradeValueCommonData>
> = {
    side: Side,
    priceType: PriceType,
    instrumentType: InstrumentType,
};

export const stringEnumSelectType: Record<
    string,
    Record<string, KungfuApi.KfTradeValueCommonData>
> = {
    exchange: ExchangeIds,
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
    extConfigs: { value: KungfuApi.KfExtConfigs };
    extTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
    mdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
} => {
    const app = getCurrentInstance();
    const extConfigs = reactive<{ value: KungfuApi.KfExtConfigs }>({
        value: {},
    });
    const extTypeMap = computed(() => buildExtTypeMap(extConfigs.value, 'td'));
    const mdExtTypeMap = computed(() =>
        buildExtTypeMap(extConfigs.value, 'md'),
    );

    onMounted(() => {
        if (app?.proxy) {
            const store = storeToRefs(app?.proxy.$useGlobalStore());
            extConfigs.value = store.extConfigs as KungfuApi.KfExtConfigs;
        }
    });

    return {
        extConfigs,
        extTypeMap,
        mdExtTypeMap,
    };
};

export const useProcessStatusDetailData = (): {
    processStatusData: Ref<Pm2ProcessStatusData>;
    processStatusDetailData: Ref<Pm2ProcessStatusDetailData>;
    appStates: Ref<Record<string, BrokerStateStatusTypes>>;
    getProcessStatusName(
        kfConfig: KungfuApi.KfConfig | KungfuApi.KfLocation,
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
                processStatusData as Pm2ProcessStatusData;
            allProcessStatusData.processStatusDetailData =
                processStatusWithDetail as Pm2ProcessStatusDetailData;
            allProcessStatusData.appStates = appStates as Record<
                string,
                BrokerStateStatusTypes
            >;
        }
    });

    const getProcessStatusName = (
        kfConfig: KungfuApi.KfConfig | KungfuApi.KfLocation,
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

            allKfConfigData.md = mdList as KungfuApi.KfConfig[];
            allKfConfigData.td = tdList as KungfuApi.KfConfig[];
            allKfConfigData.strategy = strategyList as KungfuApi.KfConfig[];
        }
    });

    return allKfConfigData;
};

export const useCurrentGlobalKfLocation = (
    watcher: KungfuApi.Watcher | null,
): {
    currentGlobalKfLocation: {
        value: KungfuApi.KfConfig | KungfuApi.KfLocation | null;
    };
    currentCategoryData: ComputedRef<KungfuApi.KfTradeValueCommonData | null>;
    currentUID: ComputedRef<string>;
    setCurrentGlobalKfLocation(kfConfig: KungfuApi.KfConfig): void;
    dealRowClassName(kfConfig: KungfuApi.KfConfig): string;
    customRow(kfConfig: KungfuApi.KfConfig): {
        onClick(): void;
    };
} => {
    const app = getCurrentInstance();
    const currentKfLocation = reactive<{
        value: KungfuApi.KfConfig | KungfuApi.KfLocation | null;
    }>({
        value: null,
    });

    onMounted(() => {
        if (app?.proxy) {
            const { currentGlobalKfLocation } = storeToRefs(
                app?.proxy.$useGlobalStore(),
            );

            currentKfLocation.value = currentGlobalKfLocation as
                | KungfuApi.KfConfig
                | KungfuApi.KfLocation
                | null;
        }
    });

    const setCurrentGlobalKfLocation = (kfLocation: KungfuApi.KfConfig) => {
        if (app?.proxy) {
            app?.proxy
                ?.$useGlobalStore()
                .setCurrentGlobalKfLocation(kfLocation);
        }
    };

    const dealRowClassName = (record: KungfuApi.KfConfig): string => {
        if (currentKfLocation.value === null) return '';
        if (
            getIdByKfLocation(record) ===
            getIdByKfLocation(currentKfLocation.value)
        ) {
            return 'current-global-kfLocation';
        }

        return '';
    };

    const customRow = (record: KungfuApi.KfConfig) => {
        return {
            onClick: () => {
                setCurrentGlobalKfLocation(record);
            },
        };
    };

    const currentCategoryData = computed(() => {
        if (!currentKfLocation.value) {
            return null;
        }

        return dealCategory(currentKfLocation.value.category);
    });

    const currentUID = computed(() => {
        if (!watcher) {
            return '';
        }

        if (!currentKfLocation.value) {
            return '';
        }

        return watcher.getLocationUID(currentKfLocation.value);
    });

    return {
        currentGlobalKfLocation: currentKfLocation,
        currentCategoryData,
        currentUID,
        setCurrentGlobalKfLocation,
        dealRowClassName,
        customRow,
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
        //如果上一步中有活动窗口，则根据当前活动窗口的右下方设置下一个窗口的坐标
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
            app?.proxy.$bus.next({
                tag: 'main',
                name: args,
            } as MainProcessEvent);
        }
    });
};

export const markClearJournal = (): void => {
    localStorage.setItem('clearJournalTradingDate', '');
    message.success('清理 journal 完成，请重启应用');
};

export const handleOpenLogview = (
    config: KungfuApi.KfConfig,
): Promise<Electron.BrowserWindow> => {
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
    assets: { value: Record<string, KungfuApi.Asset> };
    getAssetsByKfConfig(
        kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
    ): KungfuApi.Asset;
} => {
    const assetsResolved = reactive<{ value: Record<string, KungfuApi.Asset> }>(
        {
            value: {},
        },
    );

    const app = getCurrentInstance();

    onMounted(() => {
        if (app?.proxy) {
            const { assets } = storeToRefs(app?.proxy.$useGlobalStore());
            assetsResolved.value = assets;
        }
    });

    const getAssetsByKfConfig = (
        kfConfig: KungfuApi.KfConfig | KungfuApi.KfLocation,
    ): KungfuApi.Asset => {
        const processId = getProcessIdByKfLocation(kfConfig);
        return assetsResolved.value[processId] || {};
    };

    return {
        assets: assetsResolved,
        getAssetsByKfConfig,
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
            app?.proxy.$bus.next({
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

export const getIfSaveInstruments = (
    newInstruments: KungfuApi.Instrument[],
    oldInstrumentsLength: number,
): boolean => {
    if (
        newInstruments.length !== 0 &&
        newInstruments.length !== oldInstrumentsLength
    ) {
        return true;
    }

    const instrumentsSavedDate = localStorage.getItem('instrumentsSavedDate');
    if (!instrumentsSavedDate) {
        return true;
    } else if (instrumentsSavedDate !== dayjs().format('YYYY-MM-DD')) {
        return true;
    } else {
        return false;
    }
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

export const useQuote = (): {
    quotes: Ref<Record<string, KungfuApi.Quote>>;
    getQuoteByInstrument(
        instrument: KungfuApi.InstrumentResolved | undefined,
    ): KungfuApi.Quote | null;
    getLastPricePercent(
        instrument: KungfuApi.InstrumentResolved | undefined,
    ): string;
} => {
    const quotes = ref<Record<string, KungfuApi.Quote>>({});
    const app = getCurrentInstance();

    onMounted(() => {
        if (app?.proxy) {
            app.proxy.$tradingDataSubject.subscribe(
                (watcher: KungfuApi.Watcher) => {
                    quotes.value = toRaw({ ...watcher.ledger.Quote });
                },
            );
        }
    });

    const getQuoteByInstrument = (
        instrument: KungfuApi.InstrumentResolved | undefined,
    ): KungfuApi.Quote | null => {
        if (!instrument) {
            return null;
        }

        const { ukey } = instrument;
        const quote = quotes.value[ukey] as KungfuApi.Quote | undefined;
        return quote || null;
    };

    const getLastPricePercent = (
        instrument: KungfuApi.InstrumentResolved,
    ): string => {
        const quote = getQuoteByInstrument(instrument);

        if (!quote) {
            return '--';
        }

        const { open_price, last_price } = quote;
        if (!open_price || !last_price) {
            return '--';
        }

        const percent = (last_price - open_price) / open_price;
        return Number(percent * 100).toFixed(2) + '%';
    };

    return {
        quotes,
        getQuoteByInstrument,
        getLastPricePercent,
    };
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
            app?.proxy.$bus.next({
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
            app?.proxy.$bus.next({
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
            app?.proxy.$bus.next({
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
