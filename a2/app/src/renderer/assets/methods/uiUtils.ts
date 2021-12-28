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
    getMdTdKfLocationByProcessId,
    getProcessIdByKfLocation,
    getTradingDate,
    kfLogger,
    removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    AbleSubscribeInstrumentTypesBySourceType,
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
    InstrumentTypes,
    KfConfigValue,
    KfExtConfigs,
    KfTradeValueCommonData,
    AntInKungfuColorTypes,
    BrokerStateStatusTypes,
    ProcessStatusTypes,
    KfConfigItem,
} from '@kungfu-trader/kungfu-js-api/typings';

import {
    Pm2ProcessStatusData,
    Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { storeToRefs } from 'pinia';
import { BrowserWindow, getCurrentWindow } from '@electron/remote';
import { ipcRenderer } from 'electron';
import { message } from 'ant-design-vue';
import dayjs from 'dayjs';
import { kfRequestMarketData } from '@kungfu-trader/kungfu-js-api/kungfu';

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
    configSettings: KfConfigItem[],
    initValue?: Record<string, KfConfigValue>,
): Record<string, KfConfigValue> => {
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
    const formState: Record<string, KfConfigValue> = {};
    configSettings.forEach((item) => {
        const type = item.type;
        const isBoolean = booleanType.includes(type);
        const isNumber = numberType.includes(type);

        let defaultValue = item?.default;
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

export const numberEnumInputType: {
    [prop: string]: Record<number, KfTradeValueCommonData>;
} = {
    side: Side,
    offset: Offset,
    direction: Direction,
    priceType: PriceType,
    hedgeFlag: HedgeFlag,
    volumeCondition: VolumeCondition,
    timeCondition: TimeCondition,
    commissionMode: CommissionMode,
    instrumentType: InstrumentType,
};

export const stringEnumInputType: {
    [prop: string]: Record<string, KfTradeValueCommonData>;
} = {
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
): AntInKungfuColorTypes => {
    return getInstrumentTypeData(type).color || 'default';
};

export const useExtConfigsRelated = (): {
    extConfigs: { value: KfExtConfigs };
    extTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
    mdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
} => {
    const app = getCurrentInstance();
    const extConfigs = reactive<{ value: KfExtConfigs }>({
        value: {},
    });
    const extTypeMap = computed(() => buildExtTypeMap(extConfigs.value, 'td'));
    const mdExtTypeMap = computed(() =>
        buildExtTypeMap(extConfigs.value, 'md'),
    );

    onMounted(() => {
        if (app?.proxy) {
            const store = storeToRefs(app?.proxy.$useGlobalStore());
            extConfigs.value = store.extConfigs as KfExtConfigs;
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
        kfConfig: KfConfig | KfLocation,
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

    const getProcessStatusName = (kfConfig: KfConfig | KfLocation) => {
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

export const useAllKfConfigData = (): Record<KfCategoryTypes, KfConfig[]> => {
    const app = getCurrentInstance();
    const allKfConfigData: Record<KfCategoryTypes, KfConfig[]> = reactive({
        system: ref<KfConfig[]>([
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
                  ] as KfConfig[])
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

            allKfConfigData.md = mdList as KfConfig[];
            allKfConfigData.td = tdList as KfConfig[];
            allKfConfigData.strategy = strategyList as KfConfig[];
        }
    });

    return allKfConfigData;
};

export const useCurrentGlobalKfLocation = (
    watcher: Watcher | null,
): {
    currentGlobalKfLocation: {
        value: KfConfig | KfLocation | null;
    };
    currentCategoryData: ComputedRef<KfTradeValueCommonData | null>;
    currentUID: ComputedRef<string>;
    setCurrentGlobalKfLocation(kfConfig: KfConfig): void;
    dealRowClassName(kfConfig: KfConfig): string;
    customRow(kfConfig: KfConfig): {
        onClick(): void;
    };
} => {
    const app = getCurrentInstance();
    const currentKfLocation = reactive<{
        value: KfConfig | KfLocation | null;
    }>({
        value: null,
    });

    onMounted(() => {
        if (app?.proxy) {
            const { currentGlobalKfLocation } = storeToRefs(
                app?.proxy.$useGlobalStore(),
            );

            currentKfLocation.value = currentGlobalKfLocation as
                | KfConfig
                | KfLocation
                | null;
        }
    });

    const setCurrentGlobalKfLocation = (kfLocation: KfConfig) => {
        if (app?.proxy) {
            app?.proxy
                ?.$useGlobalStore()
                .setCurrentGlobalKfLocation(kfLocation);
        }
    };

    const dealRowClassName = (record: KfConfig): string => {
        if (currentKfLocation.value === null) return '';
        if (
            getIdByKfLocation(record) ===
            getIdByKfLocation(currentKfLocation.value)
        ) {
            return 'current-global-kfLocation';
        }

        return '';
    };

    const customRow = (record: KfConfig) => {
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
    config: KfConfig,
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
    assets: { value: Record<string, Asset> };
    getAssetsByKfConfig(
        kfLocation: KfLocation,
    ): Asset | Record<string, unknown>;
} => {
    const assetsResolved = reactive<{ value: Record<string, Asset> }>({
        value: {},
    });

    const app = getCurrentInstance();

    onMounted(() => {
        if (app?.proxy) {
            const { assets } = storeToRefs(app?.proxy.$useGlobalStore());
            assetsResolved.value = assets;
        }
    });

    const getAssetsByKfConfig = (kfConfig: KfConfig | KfLocation) => {
        const processId = getProcessIdByKfLocation(kfConfig);
        return assetsResolved.value[processId] || {};
    };

    return {
        assets: assetsResolved,
        getAssetsByKfConfig,
    };
};

export const getKfLocationUID = (kfConfig: KfConfig): string => {
    if (!window.watcher) return '';
    return window.watcher?.getLocationUID(kfConfig);
};

export const useDownloadHistoryTradingData = (): {
    handleDownload: (
        tradingDataType: TradingDataTypeName | 'all',
        currentKfLocation: KfLocation | KfConfig | null,
    ) => void;
} => {
    const app = getCurrentInstance();

    const handleDownload = (
        tradingDataType: TradingDataTypeName | 'all',
        currentKfLocation: KfLocation | KfConfig | null,
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
    newInstruments: Instrument[],
    oldInstrumentsLength: number,
): boolean => {
    if (newInstruments.length !== oldInstrumentsLength) {
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

export const useInstruments = (): {
    instruments: { value: InstrumentResolved[] };
    subscribedInstruments: { value: InstrumentResolved[] };
    subscribeAllInstrumentByMdProcessId(
        processId: string,
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: InstrumentResolved[],
    ): void;
    subscribeAllInstrumentByAppStates(
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: InstrumentResolved[],
    ): void;
} => {
    const app = getCurrentInstance();
    const instrumentsResolved = reactive<{ value: InstrumentResolved[] }>({
        value: [],
    });

    const subscribedInstrumentsResolved = reactive<{
        value: InstrumentResolved[];
    }>({
        value: [],
    });

    onMounted(() => {
        if (app?.proxy) {
            const { instruments, subscribedInstruments } = storeToRefs(
                app?.proxy.$useGlobalStore(),
            );
            instrumentsResolved.value = instruments as InstrumentResolved[];
            subscribedInstrumentsResolved.value =
                subscribedInstruments as InstrumentResolved[];
        }
    });

    const subscribeAllInstrumentByMdProcessId = (
        processId: string,
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: InstrumentResolved[],
    ): void => {
        if (appStates[processId] === 'Ready') {
            if (processStatus[processId] === 'online') {
                if (processId.indexOf('md_') === 0) {
                    const mdLocation = getMdTdKfLocationByProcessId(processId);
                    if (mdLocation && mdLocation.category === 'md') {
                        const sourceId = mdLocation.group;
                        const sourceType = mdExtTypeMap[sourceId];
                        const ableSubscribedInstrumentTypes =
                            AbleSubscribeInstrumentTypesBySourceType[
                                sourceType
                            ];

                        instrumentsForSubscribe.forEach((item) => {
                            if (
                                ableSubscribedInstrumentTypes.includes(
                                    +item.instrumentType,
                                )
                            ) {
                                kfRequestMarketData(
                                    window.watcher,
                                    item.exchangeId,
                                    item.instrumentId,
                                    mdLocation,
                                );
                            }
                        });
                    }
                }
            }
        }
    };

    const subscribeAllInstrumentByAppStates = (
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: InstrumentResolved[],
    ) => {
        Object.keys(appStates || {}).forEach((processId) => {
            subscribeAllInstrumentByMdProcessId(
                processId,
                processStatus,
                appStates,
                mdExtTypeMap,
                instrumentsForSubscribe,
            );
        });
    };

    return {
        instruments: instrumentsResolved,
        subscribedInstruments: subscribedInstrumentsResolved,
        subscribeAllInstrumentByMdProcessId,
        subscribeAllInstrumentByAppStates,
    };
};

export const useQuote = (): {
    quotes: Ref<Record<string, Quote>>;
    getQuoteByInstrument(
        instrument: InstrumentResolved | undefined,
    ): Quote | null;
    getLastPricePercent(instrument: InstrumentResolved | undefined): string;
} => {
    const quotes = ref<Record<string, Quote>>({});
    const app = getCurrentInstance();

    onMounted(() => {
        if (app?.proxy) {
            app.proxy.$tradingDataSubject.subscribe((watcher: Watcher) => {
                quotes.value = toRaw({ ...watcher.ledger.Quote });
            });
        }
    });

    const getQuoteByInstrument = (
        instrument: InstrumentResolved | undefined,
    ): Quote | null => {
        if (!instrument) {
            return null;
        }

        const { ukey } = instrument;
        const quote = quotes.value[ukey] as Quote | undefined;
        return quote || null;
    };

    const getLastPricePercent = (instrument: InstrumentResolved): string => {
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

export const useTriggerOrderBook = (): {
    currentInstrument: Ref<InstrumentResolved | undefined>;
    customRow(instrument: InstrumentResolved): { onClick(): void };
    triggerOrderBook(instrument: InstrumentResolved): void;
} => {
    const app = getCurrentInstance();
    const currentInstrument = ref<InstrumentResolved | undefined>();

    const triggerOrderBook = (instrument: InstrumentResolved) => {
        if (app?.proxy) {
            app?.proxy.$bus.next({
                tag: 'orderbook',
                instrument,
            });
        }
    };

    const customRow = (record: InstrumentResolved) => {
        return {
            onClick: () => {
                triggerOrderBook(record);
            },
        };
    };

    onMounted(() => {
        if (app?.proxy) {
            app.proxy.$bus.subscribe((data: KfBusEvent) => {
                if (data.tag === 'orderbook') {
                    currentInstrument.value = data.instrument;
                }
            });
        }
    });

    return {
        currentInstrument,
        customRow,
        triggerOrderBook,
    };
};
