<script setup lang="ts">
import { ref, reactive, computed, onMounted, getCurrentInstance } from 'vue';
import { storeToRefs } from 'pinia';
import dayjs from 'dayjs';
import { message, Modal } from 'ant-design-vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
} from '@ant-design/icons-vue';

import {
    KfConfig,
    KfCategoryTypes,
    KfExtOriginConfig,
    SetKfConfigPayload,
    StrategyData,
    KfLocation,
} from '@kungfu-trader/kungfu-js-api/typings';

import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useResetConfigModalPayload,
    useTableSearchKeyword,
    ensureRemoveLocation,
} from '@renderer/assets/methods/uiUtils';
import { columns } from './config';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';

const app = getCurrentInstance();

const dashboardBodyHeight = ref<number>(0);
const dashboardBodyWidth = ref<number>(0);

const setStrategyModalVisible = ref<boolean>(false);
const setStrategyConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '策略',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});
const resetConfigModalPayload = useResetConfigModalPayload();

const strategyList = reactive({
    value: [],
});
const strategyIdList = computed(() => {
    return strategyList.value.map((item: KfConfig): string => item.name);
});
const strategyListResolved = computed(() => {
    return strategyList.value.map((item: KfConfig): StrategyRow => {
        const configValue = JSON.parse(item.value || '{}');

        return {
            strategyId: item.name,
            stateStatus: 'Unknown',
            processStatus: false,
            realizedPnl: 0,
            unrealizedPnl: 0,
            marketValue: 0,
            margin: 0,
            avail: 0,
            addTime: dayjs(configValue.add_time / Math.pow(10, 6)).format(
                'MM-DD HH:mm:ss',
            ),
        };
    });
});

const { searchKeyword, tableData } = useTableSearchKeyword<StrategyRow>(
    strategyListResolved,
    ['strategyId'],
);

onMounted(() => {
    if (app?.proxy) {
        const store = app?.proxy.$useGlobalStore();
        strategyList.value = storeToRefs(store).strategyList;
    }
});

function handleOpenLog(record: StrategyRow) {
    console.log(record);
}

function handleCloseConfigModal() {
    resetConfigModalPayload(setStrategyConfigPayload);
}

function handleRemoveStrategy(record: StrategyRow) {
    const strategyLocation: KfLocation = {
        category: 'strategy',
        group: 'default',
        name: record.strategyId,
        mode: 'LIVE',
    };
    ensureRemoveLocation(strategyLocation).then(() => {
        app?.proxy && app?.proxy.$useGlobalStore().setKfConfigList();
    });
}

function handleConfirmAddUpdateStrategy(strategyData: StrategyData) {
    return Modal.confirm({
        title: `添加策略 ${strategyData.strategy_id}`,
        content: `策略ID 添加成功后不可更改, 确认添加策略 ${strategyData.strategy_id}`,
        okText: '确认',
        cancelText: '取消',
        onOk() {
            const strategyLocation: KfLocation = {
                category: 'strategy',
                group: 'default',
                name: strategyData.strategy_id,
                mode: 'LIVE',
            };

            return setKfConfig(
                strategyLocation,
                JSON.stringify({
                    ...strategyData,
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
                });
        },
    });
}

function handleOpenSetStrategyDialog(
    type: 'add' | 'update',
    record?: StrategyRow,
) {
    setStrategyConfigPayload.value.type = type;
    setStrategyConfigPayload.value.config = {
        type: [],
        settings: [
            {
                key: 'strategy_id',
                name: '策略ID',
                type: 'str',
                primary: true,
                required: true,
                tip: '需保证该策略ID唯一',
            },
            {
                key: 'strategy_path',
                name: '策略路径',
                type: 'file',
                required: true,
            },
        ],
    };

    if (type === 'update' && (record as StrategyRow).strategyId) {
        const strategyId = (record as StrategyRow).strategyId;
        const strategyConfig = findTargetFromArray<KfConfig>(
            strategyList.value,
            'name',
            strategyId,
        );

        if (strategyConfig) {
            setStrategyConfigPayload.value.initValue = JSON.parse(
                strategyConfig.value,
            );
        }
    }

    setStrategyModalVisible.value = true;
}

function handleBodySizeChange({
    width,
    height,
}: {
    width: number;
    height: number;
}) {
    const tableHeaderHeight = 36;
    dashboardBodyHeight.value = height - tableHeaderHeight;
    dashboardBodyWidth.value = width > 800 ? 800 : width;
}

function getStateStatusName(processId: string) {
    processId;
    return 'Unknown';
}
</script>

<template>
    <div class="kf-strategy__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleOpenSetStrategyDialog('add')"
                    >
                        添加
                    </a-button>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-switch></a-switch>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
            </template>
            <a-table
                ref="table"
                :columns="columns"
                :data-source="tableData"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template
                    #bodyCell="{
                        column,
                        record,
                    }: {
                        column: AntTableColumn,
                        record: StrategyRow,
                    }"
                >
                    <template v-if="column.dataIndex === 'stateStatus'">
                        <KfProcessStatus
                            :status-name="getStateStatusName(record.strategyId)"
                        ></KfProcessStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch size="small" :checked="true"></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLog(record)"
                            />
                            <SettingOutlined
                                style="font-size: 12px"
                                @click="
                                    handleOpenSetStrategyDialog(
                                        'update',
                                        record,
                                    )
                                "
                            />
                            <DeleteOutlined
                                style="font-size: 12px"
                                @click="handleRemoveStrategy(record)"
                            />
                        </div>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
        <KfSetByConfigModal
            v-if="setStrategyModalVisible"
            :width="420"
            v-model:visible="setStrategyModalVisible"
            :payload="setStrategyConfigPayload"
            :primaryKeyCompareTarget="strategyIdList"
            @confirm="handleConfirmAddUpdateStrategy"
            @close="handleCloseConfigModal"
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
export default {
    name: '策略进程',
};
</script>

<style lang="less">
.kf-strategy__warp {
    height: 100%;
}
</style>
