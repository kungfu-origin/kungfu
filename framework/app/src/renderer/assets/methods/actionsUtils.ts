import os from 'os';
import fse from 'fs-extra';
import { dialog, shell } from '@electron/remote';
import { deleteAllByKfLocation } from '@kungfu-trader/kungfu-js-api/actions';
import {
    dealTradingDataItem,
    getKungfuHistoryData,
    kfRequestMarketData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
    BrokerStateStatusTypes,
    HistoryDateEnum,
    InstrumentTypeEnum,
    InstrumentTypes,
    KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
    getCategoryData,
    getIdByKfLocation,
    getMdTdKfLocationByProcessId,
    getProcessIdByKfLocation,
    switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { writeCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { Pm2ProcessStatusData } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { message, Modal } from 'ant-design-vue';
import path from 'path';
import { Proc } from 'pm2';
import {
    computed,
    ComputedRef,
    getCurrentInstance,
    h,
    nextTick,
    onMounted,
    reactive,
    ref,
    Ref,
} from 'vue';
import dayjs from 'dayjs';
import { Row } from '@fast-csv/format';
import { KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { AbleSubscribeInstrumentTypesBySourceType } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    buildInstrumentSelectOptionLabel,
    buildInstrumentSelectOptionValue,
} from './uiUtils';
import { storeToRefs } from 'pinia';

export const ensureRemoveLocation = (
    kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void> => {
    const categoryName = getCategoryData(kfLocation.category).name;
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
                        message.error('操作失败', err.message);
                    });
            },
            onCancel() {
                resolve();
            },
        });
    });
};

export const handleSwitchProcessStatus = (
    checked: boolean,
    kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void | Proc> => {
    return switchKfLocation(window.watcher, kfLocation, checked)
        .then(() => {
            message.success('操作成功');
        })
        .catch((err: Error) => {
            message.error(err.message || '操作失败');
        });
};

export const preQuitTasks = (tasks: Promise<void>[]): Promise<[]> => {
    return Promise.all(tasks).then(() => {
        return [];
    });
};

export const useSwitchAllConfig = (
    kfConfigs: Ref<KungfuApi.KfConfig[]> | Ref<KungfuApi.KfLocation[]>,
    processStatusData: Ref<Pm2ProcessStatusData>,
): {
    allProcessOnline: ComputedRef<boolean>;
    handleSwitchAllProcessStatus(checked: boolean): Promise<void>;
} => {
    const allProcessOnline = computed(() => {
        const onlineItemsCount: number = kfConfigs.value.filter(
            (item: KungfuApi.KfConfig | KungfuApi.KfLocation): boolean => {
                const processId = getProcessIdByKfLocation(item);
                return processStatusData.value[processId] === 'online';
            },
        ).length;
        if (
            onlineItemsCount === kfConfigs.value.length &&
            kfConfigs.value.length !== 0
        ) {
            return true;
        } else {
            return false;
        }
    });

    const handleSwitchAllProcessStatus = (checked: boolean): Promise<void> => {
        return Promise.all(
            kfConfigs.value.map(
                (
                    item: KungfuApi.KfConfig | KungfuApi.KfLocation,
                ): Promise<void | Proc> => {
                    return switchKfLocation(window.watcher, item, checked);
                },
            ),
        )
            .then(() => {
                message.success('操作成功');
            })
            .catch((err: Error) => {
                message.error(err.message || '操作失败');
            });
    };

    return {
        allProcessOnline,
        handleSwitchAllProcessStatus,
    };
};

export const useAddUpdateRemoveKfConfig = (): {
    handleRemoveKfConfig: (kfConfig: KungfuApi.KfConfig) => Promise<void>;
    handleConfirmAddUpdateKfConfig: (
        data: {
            formState: Record<string, KungfuApi.KfConfigValue>;
            idByPrimaryKeys: string;
            changeType: KungfuApi.ModalChangeType;
        },
        category: KfCategoryTypes,
        group: string,
    ) => Promise<void>;
} => {
    const app = getCurrentInstance();

    const handleRemoveKfConfig = (
        kfConfig: KungfuApi.KfConfig,
    ): Promise<void> => {
        return ensureRemoveLocation(kfConfig).then(() => {
            app?.proxy && app?.proxy.$useGlobalStore().setKfConfigList();
        });
    };

    const handleConfirmAddUpdateKfConfig = (
        data: {
            formState: Record<string, KungfuApi.KfConfigValue>;
            idByPrimaryKeys: string;
            changeType: KungfuApi.ModalChangeType;
        },
        category: KfCategoryTypes,
        group: string,
    ): Promise<void> => {
        const { formState, idByPrimaryKeys, changeType } = data;

        const changeTypename = changeType === 'add' ? '添加' : '设置';
        const categoryName = getCategoryData(category).name;

        const context =
            changeType === 'add'
                ? `${categoryName}ID系统唯一, ${changeTypename}成功后不可更改, 确认${changeTypename} ${idByPrimaryKeys}`
                : `确认${changeTypename} ${idByPrimaryKeys} 相关配置`;
        return new Promise((resolve) => {
            Modal.confirm({
                title: `${changeTypename}${categoryName} ${idByPrimaryKeys}`,
                content: context,
                okText: '确认',
                cancelText: '取消',
                onOk() {
                    const kfLocation: KungfuApi.KfLocation = {
                        category: category,
                        group: group,
                        name: idByPrimaryKeys.toString(),
                        mode: 'live',
                    };

                    return setKfConfig(
                        kfLocation,
                        JSON.stringify({
                            ...formState,
                            add_time: +new Date().getTime() * Math.pow(10, 6),
                        }),
                    )
                        .then(() => {
                            message.success('操作成功');
                        })
                        .then(() => {
                            app?.proxy &&
                                app?.proxy.$useGlobalStore().setKfConfigList();
                        })
                        .catch((err: Error) => {
                            message.error('操作失败 ' + err.message);
                        })
                        .finally(() => {
                            resolve();
                        });
                },
                onCancel() {
                    resolve();
                },
            });
        });
    };

    return {
        handleRemoveKfConfig,
        handleConfirmAddUpdateKfConfig,
    };
};

export const useDealExportHistoryTradingData = (): {
    exportDateModalVisible: Ref<boolean>;
    exportDataLoading: Ref<boolean>;
    exportEventData: Ref<ExportTradingDataEvent | undefined>;
    handleConfirmExportDate(formSate: {
        date: string;
        dateType: HistoryDateEnum;
    }): void;
} => {
    const app = getCurrentInstance();
    const exportDateModalVisible = ref<boolean>(false);
    const exportEventData = ref<ExportTradingDataEvent>();
    const exportDataLoading = ref<boolean>(false);

    const dealTradingDataItemResolved = (
        item: KungfuApi.TradingDataTypes,
    ): Row => {
        return dealTradingDataItem(item, window.watcher) as Row;
    };

    const handleConfirmExportDate = async (formState: {
        date: string;
        dateType: HistoryDateEnum;
    }): Promise<void> => {
        if (!exportEventData.value) {
            throw new Error('exportEventData is undefined');
        }

        const { currentKfLocation, tradingDataType } = exportEventData.value;
        const { date, dateType } = formState;
        const dateResolved = dayjs(date).format('YYYYMMDD');

        if (tradingDataType === 'all') {
            const { tradingData } = await getKungfuHistoryData(
                window.watcher,
                date,
                dateType,
                tradingDataType,
            );
            const orders = tradingData.Order.sort('update_time');
            const trades = tradingData.Trade.sort('trade_time');
            const orderStat = tradingData.OrderStat.sort('insert_time');
            const positions = tradingData.Trade.list();

            const { filePaths } = await dialog.showOpenDialog({
                properties: ['openDirectory'],
            });

            if (!filePaths) {
                return;
            }

            const targetFolder = filePaths[0];

            const ordersFilename = path.join(
                targetFolder,
                `orders-${dateResolved}`,
            );
            const tradesFilename = path.join(
                targetFolder,
                `trades-${dateResolved}`,
            );
            const orderStatFilename = path.join(
                targetFolder,
                `orderStats-${dateResolved}`,
            );
            const posFilename = path.join(targetFolder, `pos-${dateResolved}`);

            return Promise.all([
                writeCSV(ordersFilename, orders, dealTradingDataItemResolved),
                writeCSV(tradesFilename, trades, dealTradingDataItemResolved),
                writeCSV(
                    orderStatFilename,
                    orderStat,
                    dealTradingDataItemResolved,
                ),
                writeCSV(posFilename, positions, dealTradingDataItemResolved),
            ])
                .then(() => {
                    shell.showItemInFolder(ordersFilename);
                    message.success('操作成功');
                })
                .catch((err: Error) => {
                    message.error(err.message);
                });
        }

        if (!currentKfLocation) {
            return;
        }

        exportDataLoading.value = true;
        const { historyDatas } = await getKungfuHistoryData(
            window.watcher,
            date,
            dateType,
            tradingDataType,
            currentKfLocation,
        );
        exportDataLoading.value = false;

        const processId = getProcessIdByKfLocation(currentKfLocation);
        const filename: string = await dialog
            .showSaveDialog({
                title: '保存文件',
                defaultPath: path.join(
                    os.homedir(),
                    `${processId}-${tradingDataType}-${dateResolved}.csv`,
                ),
                filters: [
                    {
                        name: 'csv',
                        extensions: ['csv'],
                    },
                ],
            })
            .then(({ filePath }) => {
                return filePath || '';
            });

        if (!filename) {
            return Promise.resolve();
        }

        return writeCSV(filename, historyDatas, dealTradingDataItemResolved)
            .then(() => {
                shell.showItemInFolder(filename);
                message.success('操作成功');
            })
            .catch((err: Error) => {
                message.error(err.message);
            });
    };

    onMounted(() => {
        if (app?.proxy) {
            app.proxy.$bus.subscribe((data: KfBusEvent) => {
                if (data.tag === 'export') {
                    exportEventData.value = data;

                    if (exportEventData.value.tradingDataType !== 'all') {
                        if (exportEventData.value.tradingDataType !== 'Order') {
                            if (
                                exportEventData.value.tradingDataType !==
                                'Trade'
                            ) {
                                if (
                                    exportEventData.value.tradingDataType !==
                                    'OrderInput'
                                ) {
                                    handleConfirmExportDate({
                                        date: dayjs().format(),
                                        dateType: HistoryDateEnum.naturalDate,
                                    });
                                    return;
                                }
                            }
                        }
                    }

                    exportDateModalVisible.value = true;
                }
            });
        }
    });

    return {
        exportDateModalVisible,
        exportDataLoading,
        exportEventData,
        handleConfirmExportDate,
    };
};

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
                `未找到 ${instrument.exchangeId}_${instrument.instrumentId} `,
            ),
        );
    }

    oldInstruments.splice(removeTargetIndex, 1);
    return fse.outputJSON(KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH, oldInstruments);
};

export const showTradingDataDetail = (
    item: KungfuApi.TradingDataTypes,
    typename: string,
): void => {
    const dataResolved = dealTradingDataItem(item, window.watcher);
    const vnode = Object.keys(dataResolved || {})
        .filter((key) => {
            return dataResolved[key] !== '';
        })
        .map((key) =>
            h('div', { class: 'trading-data-detail-row' }, [
                h('span', { class: 'label' }, `${key}`),
                h('span', { class: 'value' }, `${dataResolved[key]}`),
            ]),
        );
    Modal.confirm({
        title: `${typename} 详情`,
        content: h(
            'div',
            {
                class: 'trading-data-detail__warp',
            },
            vnode,
        ),
        okText: '确认',
        cancelText: '',
    });
};

export const useInstruments = (): {
    instruments: { data: KungfuApi.InstrumentResolved[] };
    subscribedInstruments: { data: KungfuApi.InstrumentResolved[] };
    subscribeAllInstrumentByMdProcessId(
        processId: string,
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
    ): void;
    subscribeAllInstrumentByAppStates(
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
    ): void;

    searchInstrumentResult: Ref<string | undefined>;
    searchInstrumnetOptions: Ref<{ value: string; label: string }[]>;
    transformSearchInstrumentResultToInstrument(
        instrument: string,
    ): KungfuApi.InstrumentResolved | null;
    handleSearchInstrument: (value: string) => void;
    handleConfirmSearchInstrumentResult: (
        value: string,
        callback?: (value: string) => void,
    ) => void;
} => {
    const app = getCurrentInstance();
    const instrumentsResolved = reactive<{
        data: KungfuApi.InstrumentResolved[];
    }>({
        data: [],
    });

    const subscribedInstrumentsResolved = reactive<{
        data: KungfuApi.InstrumentResolved[];
    }>({
        data: [],
    });

    onMounted(() => {
        if (app?.proxy) {
            const { instruments, subscribedInstruments } = storeToRefs(
                app?.proxy.$useGlobalStore(),
            );
            instrumentsResolved.data =
                instruments as KungfuApi.InstrumentResolved[];
            subscribedInstrumentsResolved.data =
                subscribedInstruments as KungfuApi.InstrumentResolved[];
        }
    });

    const subscribeAllInstrumentByMdProcessId = (
        processId: string,
        processStatus: Pm2ProcessStatusData,
        appStates: Record<string, BrokerStateStatusTypes>,
        mdExtTypeMap: Record<string, InstrumentTypes>,
        instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
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
        instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
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

    const searchInstrumentResult = ref<string | undefined>(undefined);
    const searchInstrumnetOptions = ref<{ value: string; label: string }[]>([]);

    const handleSearchInstrument = (val: string): void => {
        searchInstrumnetOptions.value = instrumentsResolved.data
            .filter((item) => {
                return item.id.includes(val);
            })
            .slice(0, 20)
            .map((item) => ({
                value: buildInstrumentSelectOptionValue(item),
                label: buildInstrumentSelectOptionLabel(item),
            }));
    };

    const handleConfirmSearchInstrumentResult = (
        value: string,
        callback?: (value: string) => void,
    ) => {
        nextTick().then(() => {
            searchInstrumentResult.value = undefined;
        });
        callback && callback(value);
    };

    const transformSearchInstrumentResultToInstrument = (
        instrumentStr: string,
    ): KungfuApi.InstrumentResolved | null => {
        const pair = instrumentStr.split('_');
        if (pair.length !== 5) return null;
        const [exchangeId, instrumentId, instrumentType, ukey, instrumentName] =
            pair;
        return {
            exchangeId,
            instrumentId,
            instrumentType: +instrumentType as InstrumentTypeEnum,
            instrumentName,
            id: `${instrumentId}_${instrumentName}_${exchangeId}`.toLowerCase(),
            ukey,
        };
    };

    return {
        instruments: instrumentsResolved,
        subscribedInstruments: subscribedInstrumentsResolved,
        subscribeAllInstrumentByMdProcessId,
        subscribeAllInstrumentByAppStates,

        searchInstrumentResult,
        searchInstrumnetOptions,
        transformSearchInstrumentResultToInstrument,
        handleSearchInstrument,
        handleConfirmSearchInstrumentResult,
    };
};
