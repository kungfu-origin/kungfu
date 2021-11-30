<template>
    <div class="kf-td__warp kf-translateZ">
        <KfDashboard @bodyHeight="handleBodyHeightChange">
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
                :columns="columns"
                :data-source="data"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 10, x: '100%' }"
                :locale="{
                    filterConfirm: '确认',
                    filterReset: '重置',
                    emptyText: '暂无数据',
                }"
            >
                <template #headerCell="{ column }">
                    <template v-if="column.key === 'name'">
                        <span>Name</span>
                    </template>
                </template>

                <template #bodyCell="{ column, record }">
                    <template v-if="column.key === 'stateStatus'"></template>
                    <template
                        v-else-if="column.key === 'processStatus'"
                    ></template>
                    <template v-else-if="column.key === 'action'"></template>
                </template>
            </a-table>
        </KfDashboard>
    </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
export default defineComponent({
    name: '交易账户',

    components: {
        KfDashboard,
        KfDashboardItem,
    },

    setup() {
        const columns: Array<{
            title: string;
            dataIndex: string;
            key?: string;
            width?: number | string;
            sorter?: boolean;
            align?: string;
        }> = [
            {
                title: '备注',
                dataIndex: 'accountName',
                align: 'left',
            },
            {
                title: '账户',
                dataIndex: 'accountId',
                align: 'left',
                sorter: true,
            },
            {
                title: '柜台',
                dataIndex: 'sourceId',
                align: 'left',
                sorter: true,
            },
            {
                title: '状态',
                dataIndex: 'stateStatus',
                align: 'left',
                sorter: true,
            },
            {
                title: '进程',
                dataIndex: 'processStatus',
                align: 'center',
            },
            {
                title: '实盈',
                dataIndex: 'realizedPnl',
                align: 'right',
                sorter: true,
            },
            {
                title: '浮盈',
                dataIndex: 'unrealizedPnl',
                align: 'right',
                sorter: true,
            },
            {
                title: '市值',
                dataIndex: 'marketValue',
                align: 'right',
                sorter: true,
            },
            {
                title: '保证金',
                dataIndex: 'margin',
                align: 'right',
                sorter: true,
            },
            {
                title: '可用资金',
                dataIndex: 'avail',
                align: 'right',
                sorter: true,
            },
            {
                title: '操作',
                dataIndex: 'actions',
                align: 'right',
                width: 140,
            },
        ];

        const data = [
            {
                account_name: 'asd',
                account_id: 'John Brown',
                source_id: 32,
                stateStatus: 'New York No. 1 Lake Park',
                processStatus: 'sfa',
                real: 100,
                unreal: 10000,
                market_value: 5345,
                margin: 2432,
                avail: 123,
            },
        ];

        return {
            searchKeyword: ref(''),
            dashboardBodyHeight: ref(0),
            columns,
            data,
        };
    },

    methods: {
        handleBodyHeightChange(height: number) {
            this.dashboardBodyHeight = height;
        },

        hanleOnSearch(e: string) {
            console.log(e);
        },
    },
});
</script>

<style lang="less">
.kf-td__warp {
    height: 100%;
}
</style>
