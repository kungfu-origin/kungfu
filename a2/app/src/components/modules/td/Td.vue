<template>
    <div class="kf-td__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleOpenSetSourceDialog"
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
                :data-source="tdListResolved"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template #bodyCell="{ column, record }">
                    <template v-if="column.dataIndex === 'stateStatus'">
                        <KfStateStatus
                            :status-name="
                                getStateStatusName(
                                    getTdProcessId(
                                        column.accountId,
                                        column.sourceId,
                                    ),
                                )
                            "
                        ></KfStateStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch size="small" :checked="true"></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-table-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLog(record)"
                            />
                            <SettingOutlined style="font-size: 12px" />
                            <DeleteOutlined style="font-size: 12px" />
                        </div>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
        <KfSetSourceModal
            v-if="setSourceModalVisible"
            v-model:visible="setSourceModalVisible"
            sourceType="td"
            @confirm="handleSelectedSource"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setTdModalVisible"
            v-model:visible="setTdModalVisible"
            :payload="setTdConfigPayload"
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
} from '@ant-design/icons-vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfStateStatus from '@renderer/components/public/KfStateStatus.vue';
import KfSetSourceModal from '@renderer/components/public/KfSetSourceModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';

import {
    KfCategoryTypes,
    KfExtConfig,
    SetKfConfigPayload,
    StateStatusEnum,
    KfConfig,
} from '@kungfu-trader/kungfu-js-api/typings';
import { mapState } from 'pinia';
import { useGlobalStore } from '@renderer/pages/index/store/global';
import { columns } from './config';

import '@kungfu-trader/kungfu-js-api/kungfu/store';

export default defineComponent({
    name: '交易账户',

    components: {
        KfDashboard,
        KfDashboardItem,
        KfStateStatus,
        KfSetSourceModal,
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

            setSourceModalVisible: ref<boolean>(false),
            setTdModalVisible: ref<boolean>(false),
            setTdConfigPayload: ref<SetKfConfigPayload>({
                type: 'add',
                title: '交易账户',
                config: {} as KfExtConfig['config'][KfCategoryTypes],
            }),
        };
    },

    computed: {
        ...mapState(useGlobalStore, {
            extConfigs: (store) => store.extConfigs,
            tdList: (store) => store.tdList,
        }),

        tdListResolved(): TdRow[] {
            if (!this.tdList.length) {
                return [];
            }

            return this.tdList.map((item: KfConfig) => {
                const configValue: Record<string, unknown> = JSON.parse(
                    item.value || '{}',
                );
                return {
                    accountName:
                        (configValue?.account_name as string | undefined) ||
                        item.name,
                    accountId: item.name,
                    sourceId: item.group,
                    stateStatus: StateStatusEnum.Unknown,
                    processStatus: false,
                    realizedPnl: 0,
                    unrealizedPnl: 0,
                    marketValue: 0,
                    margin: 0,
                    avail: 0,
                };
            });
        },
    },

    methods: {
        handleOpenLog(record: TdRow) {
            console.log(record);
            this.$useGlobalStore().setKfExtConfigs();
        },

        handleSelectedSource(selectedSource: string) {
            this.setTdModalVisible = true;
            this.setTdConfigPayload.title = `${selectedSource} 交易账户`;
            const targetConfig = (this.extConfigs['td'] || {})[selectedSource];

            this.setTdConfigPayload.config = targetConfig;
        },

        handleOpenSetSourceDialog() {
            this.setSourceModalVisible = true;
            this.setTdConfigPayload.type = 'add';
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

        getTdProcessId(
            sourceId: TdRow['sourceId'],
            accountId: TdRow['accountId'],
        ): string {
            return `${sourceId}_${accountId}`;
        },

        getStateStatusName(processId: string) {
            processId;
            return StateStatusEnum.launching;
        },
    },
});
</script>

<style lang="less">
.kf-td__warp {
    height: 100%;
}
</style>
