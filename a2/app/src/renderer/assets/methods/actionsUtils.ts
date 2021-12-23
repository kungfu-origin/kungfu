import os from 'os';
import { dialog } from '@electron/remote';
import { deleteAllByKfLocation } from '@kungfu-trader/kungfu-js-api/actions';
import { getKungfuHistoryData } from '@kungfu-trader/kungfu-js-api/kungfu';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
    HistoryDateEnum,
    KfCategoryTypes,
    KfConfig,
    KfConfigValue,
    KfLocation,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    getCategoryData,
    getIdByKfLocation,
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
    onMounted,
    ref,
    Ref,
} from 'vue';
import dayjs from 'dayjs';

export const ensureRemoveLocation = (
    kfLocation: KfLocation | KfConfig,
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
    kfLocation: KfLocation | KfConfig,
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
    kfConfigs: Ref<KfConfig[]> | Ref<KfLocation[]>,
    processStatusData: Ref<Pm2ProcessStatusData>,
): {
    allProcessOnline: ComputedRef<boolean>;
    handleSwitchAllProcessStatus(checked: boolean): Promise<void>;
} => {
    const allProcessOnline = computed(() => {
        const onlineItemsCount: number = kfConfigs.value.filter(
            (item: KfConfig | KfLocation): boolean => {
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
                (item: KfConfig | KfLocation): Promise<void | Proc> => {
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
    handleRemoveKfConfig: (kfConfig: KfConfig) => Promise<void>;
    handleConfirmAddUpdateKfConfig: (
        data: {
            formState: Record<string, KfConfigValue>;
            idByPrimaryKeys: string;
            changeType: ModalChangeType;
        },
        category: KfCategoryTypes,
        group: string,
    ) => Promise<void>;
} => {
    const app = getCurrentInstance();

    const handleRemoveKfConfig = (kfConfig: KfConfig): Promise<void> => {
        return ensureRemoveLocation(kfConfig).then(() => {
            app?.proxy && app?.proxy.$useGlobalStore().setKfConfigList();
        });
    };

    const handleConfirmAddUpdateKfConfig = (
        data: {
            formState: Record<string, KfConfigValue>;
            idByPrimaryKeys: string;
            changeType: ModalChangeType;
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
                    const kfLocation: KfLocation = {
                        category: category,
                        group: group,
                        name: idByPrimaryKeys.toString(),
                        mode: 'LIVE',
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

    const handleConfirmExportDate = async (formState: {
        date: string;
        dateType: HistoryDateEnum;
    }): Promise<void> => {
        if (!exportEventData.value) {
            throw new Error('exportEventData is undefined');
        }

        const { currentKfLocation, tradingDataType } = exportEventData.value;
        if (!currentKfLocation) {
            return;
        }

        exportDataLoading.value = true;
        const { date, dateType } = formState;
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
                    `${processId}-${tradingDataType}.csv`,
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

        return writeCSV(filename, historyDatas);
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
