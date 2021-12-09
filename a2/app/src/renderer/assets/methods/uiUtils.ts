import fs from 'fs-extra';
import path from 'path';
import {
    Component,
    ComputedRef,
    SetupContext,
    Ref,
    ref,
    watch,
    computed,
} from 'vue';
import {
    APP_DIR,
    KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
    buildObjectFromArray,
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
    ProcessStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings';
import { message, Modal } from 'ant-design-vue';
import {
    getCategoryName,
    getIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/kungfu/utils';
import { deleteAllByKfLocation } from '@kungfu-trader/kungfu-js-api/actions';
import { Pm2ProcessStatusData } from '@kungfu-trader/kungfu-js-api/utils/processUtils';

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

export const buildExtTypeMap = (
    extConfigs: KfExtConfigs,
    category: KfCategoryTypes,
): Record<string, InstrumentTypes> => {
    const extTypeMap: Record<string, InstrumentTypes> = {};
    const targetCategoryConfig: Record<
        string,
        KfExtOriginConfig['config'][KfCategoryTypes]
    > = extConfigs[category] || {};

    Object.keys(targetCategoryConfig).forEach((extKey: string) => {
        const configInKfExtConfig = targetCategoryConfig[extKey];
        let types = configInKfExtConfig?.type || [];
        if (typeof types === 'string') {
            types = [types];
        }

        if (!types.length) {
            extTypeMap[extKey] = 'Unknown';
            return;
        }

        const primaryType = types.sort(
            (type1: InstrumentTypes, type2: InstrumentTypes) => {
                const level1 =
                    (InstrumentType[InstrumentTypeEnum[type1]] || {}).level ||
                    0;
                const level2 =
                    (InstrumentType[InstrumentTypeEnum[type2]] || {}).level ||
                    0;
                return level2 - level1;
            },
        )[0];

        extTypeMap[extKey] = primaryType;
    });

    return extTypeMap;
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

export const getStateStatusData = (
    name: ProcessStatusTypes | undefined,
): KfTradeValueCommonData | undefined => {
    return name === undefined ? StateStatus['Unknown'] : StateStatus[name];
};

export const getIfProcessOnline = (
    processStatusData: Pm2ProcessStatusData,
    processId: string,
): boolean => {
    if (processStatusData[processId]) {
        if (processStatusData[processId] === 'online') {
            return true;
        }
    }

    return false;
};
