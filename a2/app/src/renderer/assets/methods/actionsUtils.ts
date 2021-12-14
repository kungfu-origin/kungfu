import {
    deleteAllByKfLocation,
    switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/actions';
import { KfConfig, KfLocation } from '@kungfu-trader/kungfu-js-api/typings';
import {
    getCategoryName,
    getIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { message, Modal } from 'ant-design-vue';
import { Proc } from 'pm2';

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

export const preQuitTasks = (): Promise<[]> => {
    return Promise.all([]);
};
