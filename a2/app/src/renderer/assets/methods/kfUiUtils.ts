import fs from 'fs-extra';
import path from 'path';
import {
    Component,
    ComputedRef,
    SetupContext,
    Ref,
    reactive,
    ref,
    watch,
    computed,
    getCurrentInstance,
    onMounted,
} from 'vue';
import { message, Modal } from 'ant-design-vue';

import {
    APP_DIR,
    KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
    buildExtTypeMap,
    buildObjectFromArray,
    getTradingDate,
    kfLogger,
    removeJournal,
    getCategoryName,
    getIdByKfLocation,
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
    StateStatus,
    TimeCondition,
    VolumeCondition,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    InstrumentTypeEnum,
    InstrumentTypes,
    KfCategoryTypes,
    KfConfigValue,
    KfExtOriginConfig,
    KfExtConfigs,
    KfTradeValueCommonData,
    SetKfConfigPayload,
    KfLocation,
    AntInKungfuColorTypes,
    KfConfig,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    deleteAllByKfLocation,
    switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/actions';
import {
    Pm2ProcessStatusData,
    Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { Proc } from 'pm2';
import { storeToRefs } from 'pinia';
import { BrowserWindow, getCurrentWindow } from '@electron/remote';

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

export const getStateStatusInfoByStatusType = (
    statusType: string | number,
): string => {
    // return StateStatus[statusType];
    StateStatus;
    statusType;
    return '';
};

export const useModalVisible = (
    props: { visible: boolean },
    context: SetupContext,
): { modalVisible: Ref<boolean>; closeModal: () => void } => {
    const modalVisible = ref<boolean>(props.visible);
    watch(
        () => props.visible,
        (visible) => {
            modalVisible.value = visible;
        },
    );
    const closeModal = () => {
        context.emit('update:visible', false);
        context.emit('close');
    };

    return {
        modalVisible,
        closeModal,
    };
};

export const useResetConfigModalPayload = () => {
    return (targetPayload: Ref<SetKfConfigPayload>): void => {
        targetPayload.value.title = '';
        targetPayload.value.type = 'add';
        targetPayload.value.config =
            {} as KfExtOriginConfig['config'][KfCategoryTypes];
        targetPayload.value.initValue = {};
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
                            (item as Record<string, unknown>)[key] as
                                | string
                                | number
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

export const initFormDataByConfig = (
    config: SetKfConfigPayload['config'],
    initValue?: Record<string, KfConfigValue>,
): Record<string, KfConfigValue> => {
    const settings = config?.settings;
    if (!settings) return {};

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
    settings.forEach((item) => {
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

export const ensureRemoveLocation = (kfLocation: KfLocation): Promise<void> => {
    const categoryName = getCategoryName(kfLocation);
    const id = getIdByKfLocation(kfLocation);
    return new Promise((resolve) => {
        Modal.confirm({
            title: `删除${categoryName} ${id}`,
            content: `删除${categoryName} ${id} 所有数据, 如果该${categoryName}进程正在运行, 也将停止进程, 确认删除`,
            okText: '确认',
            cancelText: '取消',
            onOk() {
                return deleteAllByKfLocation(kfLocation)
                    .then(() => {
                        message.success('操作成功');
                    })
                    .then(() => {
                        resolve();
                    })
                    .catch((err) => {
                        console.log(err);
                        message.error('操作失败', err.message);
                    });
            },
            onCancel() {
                resolve();
            },
        });
    });
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
    return InstrumentType[InstrumentTypeEnum[type]].color || 'default';
};

export const handleSwitchProcessStatus = (
    checked: boolean,
    kfLocation: KfLocation,
): Promise<void | Proc> => {
    return switchKfLocation(kfLocation, checked)
        .then(() => {
            message.success('操作成功');
        })
        .catch((err: Error) => {
            message.error(err.message || '操作失败');
        });
};

export const getExtConfigsRelated = (): {
    extConfigs: { value: KfExtConfigs };
    extTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
} => {
    const extConfigs = reactive<{ value: KfExtConfigs }>({
        value: {},
    });
    const extTypeMap = computed(() => buildExtTypeMap(extConfigs.value, 'td'));

    onMounted(() => {
        const app = getCurrentInstance();
        if (app?.proxy) {
            const store = storeToRefs(app?.proxy.$useGlobalStore());
            extConfigs.value = store.extConfigs as KfExtConfigs;
        }
    });

    return {
        extConfigs,
        extTypeMap,
    };
};

export const getProcessStatusDetailData = (): {
    processStatusData: Pm2ProcessStatusData;
    processStatusDetailData: Pm2ProcessStatusDetailData;
} => {
    const allProcessStatusData = reactive({
        processStatusData: {},
        processStatusDetailData: {},
    });

    onMounted(() => {
        const app = getCurrentInstance();
        if (app?.proxy) {
            const { processStatusData, processStatusWithDetail } = storeToRefs(
                app?.proxy.$useGlobalStore(),
            );
            allProcessStatusData.processStatusData =
                processStatusData as Pm2ProcessStatusData;
            allProcessStatusData.processStatusDetailData =
                processStatusWithDetail as Pm2ProcessStatusDetailData;
        }
    });

    return allProcessStatusData;
};

export const getAllKfConfigData = (): Record<KfCategoryTypes, KfConfig[]> => {
    const allKfConfigData: Record<KfCategoryTypes, KfConfig[]> = reactive({
        system: [
            ...(process.env.NODE_ENV === 'development'
                ? ([
                      {
                          location_uid: 0,
                          category: 'system',
                          group: 'service',
                          name: 'archive',
                          mode: 'LIVE',
                          value: '',
                      },
                  ] as KfConfig[])
                : []),
            {
                location_uid: 0,
                category: 'system',
                group: 'master',
                name: 'master',
                mode: 'LIVE',
                value: '',
            },
            {
                location_uid: 0,
                category: 'system',
                group: 'service',
                name: 'ledger',
                mode: 'LIVE',
                value: '',
            },
        ],

        md: [],
        td: [],
        strategy: [],
    });

    onMounted(() => {
        const app = getCurrentInstance();
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
            show: false,
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
