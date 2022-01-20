<script setup lang="ts">
import {
  ref,
  computed,
  toRefs,
  getCurrentInstance,
  reactive,
  onMounted,
  toRaw,
} from 'vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import KfSetSourceModal from '@renderer/components/public/KfSetSourceModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';

import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { categoryRegisterConfig, getColumns } from './config';
import {
  useTableSearchKeyword,
  useExtConfigsRelated,
  useAllKfConfigData,
  useProcessStatusDetailData,
  handleOpenLogview,
  useDashboardBodySize,
  getInstrumentTypeColor,
  useAssets,
  useCurrentGlobalKfLocation,
  useTdGroups,
  isInTdGroup,
} from '@renderer/assets/methods/uiUtils';
import {
  useAddUpdateRemoveKfConfig,
  handleSwitchProcessStatus,
  useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';
import {
  dealAssetPrice,
  getIdByKfLocation,
  getIfProcessRunning,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { message, Modal } from 'ant-design-vue';
import KfBlinkNum from '@renderer/components/public/KfBlinkNum.vue';
import {
  addTdGroup,
  removeTdGroup,
  setTdGroup,
} from '@kungfu-trader/kungfu-js-api/actions';
import SetTdGroupModal from './SetTdGroupModal.vue';

interface TdProps {}
defineProps<TdProps>();

const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();

const app = getCurrentInstance();
const setSourceModalVisible = ref<boolean>(false);
const setTdModalVisible = ref<boolean>(false);
const setTdConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: '交易账户',
  config: {} as KungfuApi.KfExtOriginConfig['config'][KfCategoryTypes],
});

const currentSelectedSourceId = ref<string>('');
const { extConfigs, extTypeMap } = useExtConfigsRelated();
const { td } = toRefs(useAllKfConfigData());
const tdIdList = computed(() => {
  return td.value.map(
    (item: KungfuApi.KfConfig): string => `${item.group}_${item.name}`,
  );
});
const { dealRowClassName, customRow } = useCurrentGlobalKfLocation(
  window.watcher,
);

const { processStatusData, getProcessStatusName } =
  useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
  td,
  processStatusData,
);

const tdGroupDataLoaded = ref<boolean>(false);
const addTdGroupModalVisble = ref<boolean>(false);
const setTdGroupModalVisble = ref<boolean>(false);
const tdGroup = useTdGroups();
const tdGroupNames = computed(() => {
  return tdGroup.data.map((item) => item.name);
});
const addTdGroupConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: '账户组',
  config: {} as KungfuApi.KfExtOriginConfig['config'][KfCategoryTypes],
});

const { searchKeyword, tableData } = useTableSearchKeyword<
  KungfuApi.KfConfig | KungfuApi.KfExtraLocation
>(td, ['group', 'name']);

const tableDataResolved = computed(() => {
  const tdGroupResolved: Record<string, KungfuApi.KfExtraLocation> = {};
  const tdResolved: KungfuApi.KfConfig[] = [];
  const markedNameToTdGroup: Record<string, KungfuApi.KfExtraLocation> = {};
  [...tdGroup.data, ...tableData.value].forEach((item) => {
    if ('children' in item) {
      markedNameToTdGroup[item.name] = { ...item };
      tdGroupResolved[item.name] = {
        ...item,
        key: item.name,
        children: [],
      };
      return;
    }

    const accountId = `${item.group}_${item.name}`;
    const targetGroupNames = Object.keys(markedNameToTdGroup).filter((name) => {
      const tdGroup = markedNameToTdGroup[name];
      return (tdGroup.children || []).includes(accountId);
    });
    if (targetGroupNames.length) {
      const targetGroupName = targetGroupNames[0];
      tdGroupResolved[targetGroupName].children?.push(item);
      return;
    }

    tdResolved.push(item as KungfuApi.KfConfig);
  });
  return [...Object.values(tdGroupResolved), ...tdResolved];
});

const { getAssetsByKfConfig, getAssetsByTdGroup } = useAssets();
const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
  useAddUpdateRemoveKfConfig();

const columns = getColumns((dataIndex) => {
  return (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => {
    return (
      (getAssetsByKfConfig(a)[dataIndex as keyof KungfuApi.Asset] || 0) -
      (getAssetsByKfConfig(b)[dataIndex as keyof KungfuApi.Asset] || 0)
    );
  };
});

onMounted(() => {
  if (app?.proxy) {
    app.proxy.$globalCategoryRegister.register(categoryRegisterConfig);
    app.proxy
      .$useGlobalStore()
      .setTdGroups()
      .then(() => {
        tdGroupDataLoaded.value = true;
      });
  }
});

function handleOpenSetTdDialog(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedSource: string,
  tdConfig?: KungfuApi.KfConfig,
) {
  currentSelectedSourceId.value = selectedSource;
  setTdConfigPayload.value.type = type;
  setTdConfigPayload.value.title = `${selectedSource} 交易账户`;
  setTdConfigPayload.value.config = (extConfigs.data['td'] || {})[
    selectedSource
  ];
  setTdConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (tdConfig) {
      setTdConfigPayload.value.initValue = JSON.parse(tdConfig.value);
    }
  }

  if (!setTdConfigPayload.value.config?.settings?.length) {
    message.error(`配置项不存在, 请检查 ${selectedSource} package.json`);
    return;
  }

  setTdModalVisible.value = true;
}

function handleOpenSetSourceDialog() {
  setSourceModalVisible.value = true;
}

function handleOpenAddTdGroupDialog(type: KungfuApi.ModalChangeType) {
  addTdGroupConfigPayload.value.type = type;
  addTdGroupConfigPayload.value.config = {
    type: [],
    settings: [
      {
        key: 'td_group_name',
        name: '账户组名称',
        type: 'str',
        primary: true,
        required: true,
        tip: '需保证该账户组名称唯一',
      },
    ],
  };
  addTdGroupConfigPayload.value.initValue = undefined;
  addTdGroupModalVisble.value = true;
}

function handleConfirmAddUpdateTdGroup(
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  const { td_group_name } = formState;
  const newTdGroup: KungfuApi.KfExtraLocation = {
    category: 'tdGroup',
    group: 'group',
    name: td_group_name.toString(),
    mode: 'LIVE',
    children: [],
  };

  return addTdGroup(newTdGroup)
    .then(() => {
      if (app?.proxy) {
        app.proxy.$useGlobalStore().setTdGroups();
      }
    })
    .then(() => {
      message.success('操作成功');
    })
    .catch((err) => {
      message.error(err.message || '操作失败');
    });
}

function handleRemoveTdGroup(item: KungfuApi.KfExtraLocation) {
  Modal.confirm({
    title: `删除账户组 ${item.name}`,
    content: `删除账户组 ${item.name}, 不会影响改账户组下账户进程, 确认删除`,
    okText: '确认',
    cancelText: '取消',
    onOk() {
      return removeTdGroup(item.name)
        .then(() => {
          if (app?.proxy) {
            app.proxy.$useGlobalStore().setTdGroups();
          }
        })
        .then(() => {
          message.success('操作成功');
        })
        .catch((err) => {
          message.error(err.message || '操作失败');
        });
    },
  });
}

function handleRemoveTd(item: KungfuApi.KfConfig) {
  handleRemoveKfConfig(item).then(() => {
    const accountId = getIdByKfLocation(item);
    const oldGroup = isInTdGroup(tdGroup.data, accountId);
    if (oldGroup) {
      const index = oldGroup.children?.indexOf(accountId);
      oldGroup.children.splice(index, 1);
      setTdGroup(toRaw(tdGroup.data)).then(() => {
        if (app?.proxy) {
          app?.proxy.$useGlobalStore().setTdGroups();
        }
      });
    }
  });
}
</script>

<template>
  <div class="kf-td__warp kf-translateZ">
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
          <a-switch
            :checked="allProcessOnline"
            @click="handleSwitchAllProcessStatus"
          ></a-switch>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleOpenAddTdGroupDialog('add')">
            添加分组
          </a-button>
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
        v-if="tdGroupDataLoaded"
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableDataResolved"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :rowClassName="dealRowClassName"
        :customRow="customRow"
        :defaultExpandAllRows="true"
        emptyText="暂无数据"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.KfConfig | KungfuApi.KfExtraLocation,
          }"
        >
          <template
            v-if="column.dataIndex === 'name' && record.category === 'td'"
          >
            <a-tag :color="getInstrumentTypeColor(extTypeMap[record.group])">
              {{ record.group }}
            </a-tag>
            {{ record.name }}
          </template>
          <template
            v-else-if="
              column.dataIndex === 'name' && record.category === 'tdGroup'
            "
          >
            <a-tag color="#FAAD14">账户组</a-tag>
            {{ record.name }}
          </template>
          <template
            v-else-if="
              column.dataIndex === 'accountName' && record.category === 'td'
            "
          >
            {{
                            JSON.parse((record as KungfuApi.KfConfig).value)
                                .account_name || '--'
            }}
          </template>

          <template v-else-if="column.dataIndex === 'stateStatus'">
            <KfProcessStatus
              v-if="record.category === 'td'"
              :statusName="getProcessStatusName(record)"
            ></KfProcessStatus>
          </template>
          <template v-else-if="column.dataIndex === 'processStatus'">
            <a-switch
              v-if="record.category === 'td'"
              size="small"
              :checked="
                getIfProcessRunning(
                  processStatusData,
                  getProcessIdByKfLocation(record),
                )
              "
              @click="
                                (checked: boolean, Event: MouseEvent) => handleSwitchProcessStatus(checked, Event, record)
                            "
            ></a-switch>
          </template>
          <template v-else-if="column.dataIndex === 'unrealizedPnl'">
            <KfBlinkNum
              v-if="record.category === 'td'"
              mode="compare-zero"
              :num="dealAssetPrice(getAssetsByKfConfig(record).unrealized_pnl)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              mode="compare-zero"
              :num="dealAssetPrice(getAssetsByTdGroup(record).unrealized_pnl)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'marketValue'">
            <KfBlinkNum
              v-if="record.category === 'td'"
              :num="dealAssetPrice(getAssetsByKfConfig(record).market_value)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="dealAssetPrice(getAssetsByTdGroup(record).market_value)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'margin'">
            <KfBlinkNum
              v-if="record.category === 'td'"
              :num="dealAssetPrice(getAssetsByKfConfig(record).margin)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="dealAssetPrice(getAssetsByTdGroup(record).margin)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'avail'">
            <KfBlinkNum
              v-if="record.category === 'td'"
              :num="dealAssetPrice(getAssetsByKfConfig(record).avail)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="dealAssetPrice(getAssetsByTdGroup(record).avail)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp" v-if="record.category === 'td'">
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogview(record)"
              />
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="
                                    handleOpenSetTdDialog(
                                        'update',
                                        record.group,
                                        record as KungfuApi.KfConfig,
                                    )
                                "
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveTd(record)"
              />
            </div>
            <div class="kf-actions__warp" v-if="record.category === 'tdGroup'">
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="setTdGroupModalVisble = true"
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveTdGroup(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
    <KfSetSourceModal
      v-if="setSourceModalVisible"
      v-model:visible="setSourceModalVisible"
      sourceType="td"
      @confirm="handleOpenSetTdDialog('add', $event)"
    ></KfSetSourceModal>
    <KfSetByConfigModal
      v-if="setTdModalVisible"
      v-model:visible="setTdModalVisible"
      :payload="setTdConfigPayload"
      :primaryKeyAvoidRepeatCompareTarget="tdIdList"
      :primaryKeyAvoidRepeatCompareExtra="currentSelectedSourceId"
      @confirm="
        handleConfirmAddUpdateKfConfig($event, 'td', currentSelectedSourceId)
      "
    ></KfSetByConfigModal>
    <KfSetByConfigModal
      v-if="addTdGroupModalVisble"
      v-model:visible="addTdGroupModalVisble"
      :payload="addTdGroupConfigPayload"
      :primaryKeyAvoidRepeatCompareTarget="tdGroupNames"
      @confirm="({ formState }) => handleConfirmAddUpdateTdGroup(formState)"
    ></KfSetByConfigModal>
    <SetTdGroupModal
      v-if="setTdGroupModalVisble"
      v-model:visible="setTdGroupModalVisble"
    ></SetTdGroupModal>
  </div>
</template>
<style lang="less">
.kf-td__warp {
  height: 100%;
}
</style>
