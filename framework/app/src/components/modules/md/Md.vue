<script setup lang="ts">
import { computed, ref, toRefs } from 'vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfProcessStatus.vue';
import KfSetExtensionModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetExtensionModal.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';

import { columns } from './config';
import {
  getInstrumentTypeColor,
  useExtConfigsRelated,
  useProcessStatusDetailData,
  handleOpenLogview,
  useDashboardBodySize,
  useTableSearchKeyword,
  useAllKfConfigData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getIdByKfLocation,
  getIfProcessRunning,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  handleSwitchProcessStatus,
  useAddUpdateRemoveKfConfig,
  useSwitchAllConfig,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { message } from 'ant-design-vue';

interface MdProps {}
defineProps<MdProps>();

const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();

const setSourceModalVisible = ref<boolean>(false);
const setMdModalVisible = ref<boolean>(false);
const setMdConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: '行情源',
  config: {} as KungfuApi.KfExtConfig,
});
const currentSelectedSourceId = ref<string>('');

const { extConfigs, tdExtTypeMap } = useExtConfigsRelated();
const { md } = toRefs(useAllKfConfigData());
const mdIdList = computed(() => {
  return md.value.map((item: KungfuApi.KfConfig): string =>
    getIdByKfLocation(item),
  );
});
const { processStatusData, getProcessStatusName } =
  useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
  md,
  processStatusData,
);

const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.KfConfig>(
  md,
  ['group'],
);

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
  useAddUpdateRemoveKfConfig();

function handleOpenSetMdDialog(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedSource: string,
  mdConfig?: KungfuApi.KfConfig,
) {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['md'] || {})[
    selectedSource
  ];

  if (!extConfig) {
    message.error(`${selectedSource} 柜台插件不存在`);
    return;
  }

  currentSelectedSourceId.value = selectedSource;
  setMdConfigPayload.value.type = type;
  setMdConfigPayload.value.title = `${selectedSource} 行情源`;
  setMdConfigPayload.value.config = extConfig;
  setMdConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (mdConfig) {
      setMdConfigPayload.value.initValue = JSON.parse(mdConfig.value);
    }
  }

  if (!extConfig?.settings?.length) {
    handleConfirmAddUpdateKfConfig(
      {
        formState: {} as Record<string, KungfuApi.KfConfigValue>,
        idByPrimaryKeys: selectedSource,
        changeType: type,
      },
      'md',
      selectedSource,
    );
    return;
  }

  setMdModalVisible.value = true;
}

function handleOpenSetSourceDialog() {
  setSourceModalVisible.value = true;
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
        class="kf-ant-table"
        ref="table"
        :columns="columns"
        :data-source="tableData"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        emptyText="暂无数据"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.KfConfig,
          }"
        >
          <template v-if="column.dataIndex === 'name'">
            <a-tag :color="getInstrumentTypeColor(tdExtTypeMap[record.name])">
              {{ record.group }}
            </a-tag>
          </template>
          <template v-else-if="column.dataIndex === 'stateStatus'">
            <KfProcessStatus
              :statusName="getProcessStatusName(record)"
            ></KfProcessStatus>
          </template>
          <template v-else-if="column.dataIndex === 'processStatus'">
            <a-switch
              size="small"
              :checked="
                getIfProcessRunning(
                  processStatusData,
                  getProcessIdByKfLocation(record),
                )
              "
              @click="(checked: boolean, Event: MouseEvent) => handleSwitchProcessStatus(checked, Event, record)"
            ></a-switch>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogview(record)"
              />
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="
                  handleOpenSetMdDialog('update', record.group, record)
                "
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveKfConfig(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
    <KfSetExtensionModal
      v-if="setSourceModalVisible"
      v-model:visible="setSourceModalVisible"
      extensionType="md"
      @confirm="handleOpenSetMdDialog('add', $event)"
    ></KfSetExtensionModal>
    <KfSetByConfigModal
      v-if="setMdModalVisible"
      v-model:visible="setMdModalVisible"
      :payload="setMdConfigPayload"
      :primaryKeyAvoidRepeatCompareTarget="mdIdList"
      :primaryKeyAvoidRepeatCompareExtra="currentSelectedSourceId"
      @confirm="
        handleConfirmAddUpdateKfConfig(
          { ...$event, idByPrimaryKeys: currentSelectedSourceId },
          'md',
          currentSelectedSourceId,
        )
      "
    ></KfSetByConfigModal>
  </div>
</template>
<style lang="less">
.kf-md__warp {
  height: 100%;
}
</style>
