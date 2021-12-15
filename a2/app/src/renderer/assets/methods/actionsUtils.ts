import {
    deleteAllByKfLocation,
    switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/actions';
import { KfConfig, KfLocation } from '@kungfu-trader/kungfu-js-api/typings';
import {
    getCategoryName,
    getIdByKfLocation,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Pm2ProcessStatusData } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { message, Modal } from 'ant-design-vue';
import { Proc } from 'pm2';
import { computed, ComputedRef, getCurrentInstance, Ref } from 'vue';

export const ensureRemoveLocation = (
    kfLocation: KfLocation | KfConfig,
): Promise<void> => {
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

export const handleSwitchProcessStatus = (
    checked: boolean,
    kfLocation: KfLocation | KfConfig,
): Promise<void | Proc> => {
    return switchKfLocation(kfLocation, checked)
        .then(() => {
            message.success('操作成功');
        })
        .catch((err: Error) => {
            message.error(err.message || '操作失败');
        });
};

export const preQuitTasks = (): Promise<[]> => {
    return Promise.all([]).then(() => {
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
        if (onlineItemsCount === kfConfigs.value.length) {
            return true;
        } else {
            return false;
        }
    });

    const handleSwitchAllProcessStatus = (checked: boolean): Promise<void> => {
        return Promise.all(
            kfConfigs.value.map(
                (item: KfConfig | KfLocation): Promise<void | Proc> => {
                    return switchKfLocation(item, checked);
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

export const handleRemoveKfConfig = (kfConfig: KfConfig): Promise<void> => {
    return ensureRemoveLocation(kfConfig).then(() => {
        const app = getCurrentInstance();
        app?.proxy && app?.proxy.$useGlobalStore().setKfConfigList();
    });
};
