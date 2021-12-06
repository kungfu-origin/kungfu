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
                        @search="hanleOnSearch"
                    />
                </KfDashboardItem>
            </template>
            <a-table
                ref="table"
                :columns="columns"
                :data-source="strategyListResolved"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template #bodyCell="props">
                    <template
                        v-if="(props.column as AntTableColumn).dataIndex === 'stateStatus'"
                    >
                        <KfStateStatus
                            :status-name="getStateStatusName((props.record as StrategyRow).strategyId)"
                        ></KfStateStatus>
                    </template>
                    <template
                        v-else-if="(props.column as AntTableColumn).dataIndex === 'processStatus'"
                    >
                        <a-switch size="small" :checked="true"></a-switch>
                    </template>
                    <template
                        v-else-if="(props.column as AntTableColumn).dataIndex === 'actions'"
                    >
                        <div class="kf-table-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLog(props.record as StrategyRow)"
                            />
                            <SettingOutlined
                                style="font-size: 12px"
                                @click="
                                    handleOpenSetStrategyDialog(
                                        'update',
                                        (props.record as StrategyRow),
                                    )
                                "
                            />
                            <DeleteOutlined
                                style="font-size: 12px"
                                @click="handleRemoveStrategy(props.record as StrategyRow)"
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
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { mapState } from 'pinia';
import dayjs from 'dayjs';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
} from '@ant-design/icons-vue';
import { message, Modal } from 'ant-design-vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfStateStatus from '@renderer/components/public/KfStateStatus.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';

import {
    KfConfig,
    KfCategoryTypes,
    KfExtConfig,
    SetKfConfigPayload,
    StateStatusEnum,
    StrategyData,
} from '@kungfu-trader/kungfu-js-api/typings';
import type { StrategyRow } from '@kungfu-trader/kungfu-js-api/typings';

import { useGlobalStore } from '@renderer/pages/index/store/global';
import { columns } from './config';

import '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
    addStrategy,
    deleteStrategy,
} from '@kungfu-trader/kungfu-js-api/actions';
import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

dayjs.locale('zh-cn');

export default defineComponent({
    name: '策略进程',

    components: {
        KfDashboard,
        KfDashboardItem,
        KfStateStatus,
        KfSetByConfigModal,
        FileTextOutlined,
        SettingOutlined,
        DeleteOutlined,
    },

    setup() {
        return {
            searchKeyword: ref<string>(''),
            dashboardBodyHeight: ref<number>(0),
            dashboardBodyWidth: ref<number>(0),
            columns,

            setStrategyModalVisible: ref<boolean>(false),
            setStrategyConfigPayload: ref<SetKfConfigPayload>({
                type: 'add',
                title: '策略',
                config: {} as KfExtConfig['config'][KfCategoryTypes],
            }),

            StrategyRow: typeof StrategyRow,
        };
    },

    computed: {
        ...mapState(useGlobalStore, {
            extConfigs: (store) => store.extConfigs,
            strategyList: (store) => store.strategyList,
        }),

        strategyIdList(): string[] {
            return this.strategyList.map((item: KfConfig) => item.name);
        },

        strategyListResolved(): StrategyRow[] {
            if (!this.strategyList.length) {
                return [];
            }

            return this.strategyList.map((item: KfConfig) => {
                const configValue = JSON.parse(item.value || '{}');

                return {
                    strategyId: item.name,
                    stateStatus: StateStatusEnum.Unknown,
                    processStatus: false,
                    realizedPnl: 0,
                    unrealizedPnl: 0,
                    marketValue: 0,
                    margin: 0,
                    avail: 0,
                    addTime: dayjs(
                        configValue.add_time / Math.pow(10, 6),
                    ).format('MM-DD HH:mm:ss'),
                };
            });
        },
    },

    methods: {
        handleOpenLog(record: StrategyRow) {
            console.log(record);
            this.$useGlobalStore().setKfExtConfigs();
        },

        handleRemoveStrategy(record: StrategyRow) {
            const strategyId = record.strategyId;
            const self = this;
            return Modal.confirm({
                title: `删除策略 ${strategyId}`,
                content: `删除策略 ${strategyId} 所有数据，确认删除`,
                okText: '确认',
                cancelText: '取消',
                onOk() {
                    return deleteStrategy(strategyId)
                        .then(() => {
                            message.success('操作成功');
                        })
                        .then(() => {
                            self.$useGlobalStore().setKfConfigList();
                        })
                        .catch((err) => {
                            message.error('操作失败', err.message);
                        });
                },
            });
        },

        handleConfirmAddUpdateStrategy(strategyData: StrategyData) {
            const self = this;
            return Modal.confirm({
                title: `添加策略 ${strategyData.strategy_id}`,
                content: `策略ID 添加成功后不可更改, 确认添加策略 ${strategyData.strategy_id}`,
                okText: '确认',
                cancelText: '取消',
                onOk() {
                    return addStrategy(
                        strategyData.strategy_id,
                        strategyData.strategy_path,
                    )
                        .then(() => {
                            message.success('操作成功');
                        })
                        .then(() => {
                            self.$useGlobalStore().setKfConfigList();
                        })
                        .catch((err) => {
                            message.error('操作失败', err.message);
                        });
                },
            });
        },

        handleOpenSetStrategyDialog(
            type: 'add' | 'update',
            record?: StrategyRow,
        ) {
            this.setStrategyConfigPayload.type = type;

            this.setStrategyConfigPayload.config = {
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
                    this.strategyList,
                    'name',
                    strategyId,
                );

                if (strategyConfig) {
                    this.setStrategyConfigPayload.initValue = JSON.parse(
                        strategyConfig.value,
                    );
                }
            }

            this.setStrategyModalVisible = true;
        },

        handleBodySizeChange({
            width,
            height,
        }: {
            width: number;
            height: number;
        }) {
            const tableHeaderHeight = 36;
            this.dashboardBodyHeight = height - tableHeaderHeight;
            this.dashboardBodyWidth = width > 800 ? 800 : width;
        },

        hanleOnSearch(e: string) {
            console.log(e);
        },

        getStateStatusName(processId: string) {
            processId;
            return StateStatusEnum.launching;
        },
    },
});
</script>

<style lang="less">
.kf-strategy__warp {
    height: 100%;
}
</style>
