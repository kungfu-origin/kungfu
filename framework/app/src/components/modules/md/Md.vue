<script setup lang="ts">
import { computed, Ref, ref, toRefs } from 'vue';
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
  handleOpenLogview,
  messagePrompt,
  useDashboardBodySize,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getIdByKfLocation,
  getIfProcessRunning,
  getIfProcessStopping,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  handleSwitchProcessStatus,
  useAddUpdateRemoveKfConfig,
  useAllKfConfigData,
  useExtConfigsRelated,
  useProcessStatusDetailData,
  useSwitchAllConfig,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();

const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();

const setSourceModalVisible = ref<boolean>(false);
const setMdModalVisible = ref<boolean>(false);
const setMdConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('Md'),
  config: {} as KungfuApi.KfExtConfig,
});
const currentSelectedSourceId = ref<string>('');

const { extConfigs, mdExtTypeMap } = useExtConfigsRelated();
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
  md as Ref<KungfuApi.KfConfig[]>,
  ['group'],
);

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
  useAddUpdateRemoveKfConfig();

function handleOpenSetMdDialog(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedSource: string,
  mdConfig?: KungfuApi.KfConfig,
) {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['md'] || {})[
    selectedSource
  ];

  if (!extConfig) {
    error(`${selectedSource} ${t('mdConfig.counter_plugin_inexistence')}`);
    return;
  }

  currentSelectedSourceId.value = selectedSource;
  setMdConfigPayload.value.type = type;
  setMdConfigPayload.value.title = `${selectedSource} ${t('Md')}`;
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
        configSettings: [],
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

function handleRemoveMd(record: KungfuApi.KfConfig) {
  return handleRemoveKfConfig(window.watcher, record, processStatusData.value)
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
    });
}
</script>
<template>
  <div class="kf-md__warp kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
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
            {{ $t('mdConfig.add_md') }}
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
        :emptyText="$t('empty_text')"
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
            <a-tag :color="getInstrumentTypeColor(mdExtTypeMap[record.name])">
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
              :loading="
                getIfProcessStopping(
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
                @click.stop="handleRemoveMd(record)"
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
