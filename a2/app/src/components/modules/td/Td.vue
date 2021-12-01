<template>
    <div class="kf-td__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-button size="small" type="primary">添加</a-button>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button size="small">开启全部</a-button>
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
                :data-source="data"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template #headerCell="{ column }">
                    <template v-if="column.key === 'name'">
                        <span>Name</span>
                    </template>
                </template>

                <template #bodyCell="{ column, record }">
                    <template v-if="column.dataIndex === 'stateStatus'">
                        <KfStateStatus
                            :status-name="
                                getStateStatusName(
                                    getTdProcessId(
                                        record[column.accountId],
                                        record[column.sourceId],
                                    ),
                                )
                            "
                        ></KfStateStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch :checked="true"></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-table-actions__warp">
                            <FileTextOutlined style="font-size: 12px" />
                            <SettingOutlined style="font-size: 12px" />
                            <DeleteOutlined style="font-size: 12px" />
                        </div>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
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

import {
    StateStatusTypes,
    TdOptions,
} from '@kungfu-trader/kungfu-js-api/typings';

export default defineComponent({
    name: '交易账户',

    components: {
        KfDashboard,
        KfDashboardItem,
        KfStateStatus,
        FileTextOutlined,
        SettingOutlined,
        DeleteOutlined,
    },

    setup() {
        const columns: AntTableColumns = [
            {
                title: '备注',
                dataIndex: 'accountName',
                align: 'left',
                sorter: true,
                fixed: 'left',
                width: 80,
            },
            {
                title: '账户',
                dataIndex: 'accountId',
                align: 'left',
                sorter: true,
                fixed: 'left',
                width: 80,
            },
            {
                title: '柜台',
                dataIndex: 'sourceId',
                align: 'left',
                sorter: true,
                fixed: 'left',
                width: 60,
            },
            {
                title: '状态',
                dataIndex: 'stateStatus',
                align: 'left',
                sorter: true,
                width: 80,
            },
            {
                title: '进程',
                dataIndex: 'processStatus',
                align: 'center',
                sorter: true,
                width: 60,
            },
            {
                title: '浮动盈亏',
                dataIndex: 'unrealizedPnl',
                align: 'right',
                sorter: true,
                width: 90,
            },
            {
                title: '可用资金',
                dataIndex: 'avail',
                align: 'right',
                sorter: true,
                width: 90,
            },
            {
                title: '市值',
                dataIndex: 'marketValue',
                align: 'right',
                sorter: true,
                width: 90,
            },
            {
                title: '保证金',
                dataIndex: 'margin',
                align: 'right',
                sorter: true,
                width: 90,
            },
            {
                title: '操作',
                dataIndex: 'actions',
                align: 'right',
                width: 140,
                fixed: 'right',
            },
        ];

        const data: TdOptions[] = [
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
            {
                accountName: 'asd----1-1-1-1-1-1',
                accountId: 'John Brown',
                sourceId: '32',
                stateStatus: StateStatusTypes.online,
                processStatus: true,
                realizedPnl: 90,
                unrealizedPnl: 10000,
                marketValue: 5345,
                margin: 2432,
                avail: 123,
            },
        ];

        return {
            searchKeyword: ref<string>(''),
            dashboardBodyHeight: ref<number>(0),
            dashboardBodyWidth: ref<number>(0),
            tableHeight: ref<number>(0),
            columns,
            data,
        };
    },

    mounted() {},

    methods: {
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
            sourceId: TdOptions['sourceId'],
            accountId: TdOptions['accountId'],
        ): string {
            return `${sourceId}_${accountId}`;
        },

        getStateStatusName(processId: string) {
            console.log(processId);
            return StateStatusTypes.launching;
        },
    },
});
</script>

<style lang="less">
.kf-td__warp {
    height: 100%;
}
</style>
