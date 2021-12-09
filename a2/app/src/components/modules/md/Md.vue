<template>
    <div class="kf-md__warp kf-translateZ">
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
                :data-source="mdListResolved"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template #bodyCell="{ column, record }">
                    <template v-if="column.dataIndex === 'stateStatus'">
                        <KfStateStatus
                            :status-name="
                                getStateStatusName(
                                    getMdProcessId(column.sourceId),
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
            sourceType="md"
            @confirm="handleSelectedSource"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setMdModalVisible"
            v-model:visible="setMdModalVisible"
            :payload="setMdConfigPayload"
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
    KfExtOriginConfig,
    SetKfConfigPayload,
    KfConfig,
} from '@kungfu-trader/kungfu-js-api/typings';
import { mapState } from 'pinia';
import { useGlobalStore } from '@renderer/pages/index/store/global';
import { columns } from './config';

export default defineComponent({
    name: '行情源',

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
            setMdModalVisible: ref<boolean>(false),
            setMdConfigPayload: ref<SetKfConfigPayload>({
                type: 'add',
                title: '行情源',
                config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
            }),
        };
    },

    computed: {
        ...mapState(useGlobalStore, {
            extConfigs: (store) => store.extConfigs,
            mdList: (store) => store.mdList,
        }),

        mdListResolved(): MdRow[] {
            if (!this.mdList.length) {
                return [];
            }

            return this.mdList.map((item: KfConfig) => {
                return {
                    sourceId: item.name,
                    stateStatus: 'Unknown',
                    processStatus: false,
                };
            });
        },
    },

    methods: {
        handleOpenLog(record: MdRow) {
            console.log(record);
            this.$useGlobalStore().setKfExtConfigs();
        },

        handleSelectedSource(selectedSource: string) {
            this.setMdModalVisible = true;
            this.setMdConfigPayload.title = `${selectedSource} 行情源`;
            const targetConfig = (this.extConfigs['md'] || {})[selectedSource];

            this.setMdConfigPayload.config = targetConfig;
        },

        handleOpenSetSourceDialog() {
            this.setSourceModalVisible = true;
            this.setMdConfigPayload.type = 'add';
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

        getMdProcessId(sourceId: MdRow['sourceId']): string {
            return `md_${sourceId}`;
        },

        getStateStatusName(processId: string) {
            processId;
            return 'Unknown';
        },
    },
});
</script>

<style lang="less">
.kf-md__warp {
    height: 100%;
}
</style>
