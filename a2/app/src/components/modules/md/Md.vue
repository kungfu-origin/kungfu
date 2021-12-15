<script setup lang="ts">
import { computed, getCurrentInstance, ref, toRefs } from 'vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
} from '@ant-design/icons-vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import KfSetSourceModal from '@renderer/components/public/KfSetSourceModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';

import {
    KfCategoryTypes,
    KfExtOriginConfig,
    ProcessStatusTypes,
    SetKfConfigPayload,
} from '@kungfu-trader/kungfu-js-api/typings';
import type { KfConfig } from '@kungfu-trader/kungfu-js-api/typings';
import { columns } from './config';
import {
    getAllKfConfigData,
    getExtColor,
    getExtConfigsRelated,
    getProcessStatusDetailData,
    handleOpenLogview,
    useDashboardBodySize,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import {
    getIdByKfLocation,
    getIfProcessOnline,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    handleRemoveKfConfig,
    handleSwitchProcessStatus,
    useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';

const app = getCurrentInstance();

interface MdProps {}
defineProps<MdProps>();

const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
    useDashboardBodySize();

const setSourceModalVisible = ref<boolean>(false);
const setMdModalVisible = ref<boolean>(false);
const setMdConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '行情源',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});
const currentSelectedSourceId = ref<string>('');

const { extConfigs, extTypeMap } = getExtConfigsRelated();

const { md } = toRefs(getAllKfConfigData());
const mdIdList = computed(() => {
    return md.value.map((item: KfConfig): string => getIdByKfLocation(item));
});

const { processStatusData } = toRefs(getProcessStatusDetailData());

const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
    md,
    processStatusData,
);

const { searchKeyword, tableData } = useTableSearchKeyword<KfConfig>(md, [
    'group',
]);

function handleSelectedSource(selectedSource: string) {
    setMdModalVisible.value = true;
    setMdConfigPayload.value.title = `${selectedSource} 行情源`;
    const targetConfig = (extConfigs.value['md'] || {})[selectedSource];
    setMdConfigPayload.value.config = targetConfig;
    currentSelectedSourceId.value = selectedSource;
}

function handleOpenSetSourceDialog() {
    setSourceModalVisible.value = true;
    setMdConfigPayload.value.type = 'add';
}

function getStateStatusName(processId: string): ProcessStatusTypes {
    processId;
    return 'Unknown';
}
</script>
<template>
    <div class="kf-md__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
                <KfDashboardItem>
                    <KfDashboardItem>
                        <a-switch
                            :checked="allProcessOnline"
                            @click="handleSwitchAllProcessStatus"
                        ></a-switch>
                    </KfDashboardItem>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleOpenSetSourceDialog"
                    >
                        添加
                    </a-button>
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
                        record: KfConfig,
                    }"
                >
                    <template v-if="column.dataIndex === 'name'">
                        <a-tag :color="getExtColor(extTypeMap, record.name)">
                            {{ record.name }}
                        </a-tag>
                    </template>
                    <template v-else-if="column.dataIndex === 'stateStatus'">
                        <KfProcessStatus
                            :status-name="
                                getStateStatusName(
                                    getProcessIdByKfLocation(record),
                                )
                            "
                        ></KfProcessStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch
                            size="small"
                            :checked="
                                getIfProcessOnline(
                                    processStatusData,
                                    getProcessIdByKfLocation(record),
                                )
                            "
                            @click="handleSwitchProcessStatus($event, record)"
                        ></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLogview(record)"
                            />
                            <SettingOutlined style="font-size: 12px" />
                            <DeleteOutlined
                                style="font-size: 12px"
                                @click="handleRemoveKfConfig(record)"
                            />
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
            :primaryKeyCompareTarget="mdIdList"
            :primaryKeyCompareExtra="currentSelectedSourceId"
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
export default {
    name: '行情源',
};
</script>

<style lang="less">
.kf-md__warp {
    height: 100%;
}
</style>
