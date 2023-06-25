<script setup lang="ts">
import { ref, computed, toRefs, onMounted, toRaw } from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfProcessStatus.vue';
import KfSetExtensionModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetExtensionModal.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import FundTransModal from './FundTransModal.vue';
import Icon, {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
  ReloadOutlined,
  PayCircleOutlined,
} from '@ant-design/icons-vue';

import { categoryRegisterConfig, getColumns, getFundTransKey } from './config';
import {
  useTableSearchKeyword,
  handleOpenLogview,
  useDashboardBodySize,
  getInstrumentTypeColor,
  isInTdGroup,
  confirmModal,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  useAddUpdateRemoveKfConfig,
  handleSwitchProcessStatusGenerator,
  useSwitchAllConfig,
  useProcessStatusDetailData,
  useExtConfigsRelated,
  useCurrentGlobalKfLocation,
  useAllKfConfigData,
  useTdGroups,
  useAssets,
  useAssetMargins,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealAssetPrice,
  getIdByKfLocation,
  getIfProcessRunning,
  getIfProcessStopping,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import {
  addTdGroup,
  removeTdGroup,
  setTdGroup,
} from '@kungfu-trader/kungfu-js-api/actions';
import SetTdGroupModal from './SetTdGroupModal.vue';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { storeToRefs } from 'pinia';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const handleSwitchProcessStatus = handleSwitchProcessStatusGenerator();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { globalSetting } = storeToRefs(useGlobalStore());
const isShowAssetMargin = computed(() => {
  return !!globalSetting.value?.trade?.assetMargin;
});

globalThis.HookKeeper.getHooks().dealTradingData.register(
  {
    category: categoryRegisterConfig.category,
    group: '*',
    name: '*',
  },
  categoryRegisterConfig,
);

const setSourceModalVisible = ref<boolean>(false);
const setTdModalVisible = ref<boolean>(false);
const setTdConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('Td'),
  config: {} as KungfuApi.KfExtConfig,
});

const currentSelectedSourceId = ref<string>('');
const { extConfigs, tdExtTypeMap } = useExtConfigsRelated();
const { td } = toRefs(useAllKfConfigData());
const tdIdList = computed(() => {
  return td.value.map(
    (item: KungfuApi.KfLocation): string => `${item.group}_${item.name}`,
  );
});
const {
  dealRowClassName,
  customRow,
  currentGlobalKfLocation,
  resetCurrentGlobalKfLocation,
} = useCurrentGlobalKfLocation(window.watcher);

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
  return tdGroup.value.map((item) => item.name);
});
const addTdGroupConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('tdConfig.account_group'),
  config: {} as KungfuApi.KfExtConfig,
});

const currentAccout: {
  source: string;
  transfer_type: 'between_nodes' | 'tranc_in';
  config: KungfuApi.KfConfig | null;
  avail: number;
} = {
  source: '',
  transfer_type: 'between_nodes',
  config: null,
  avail: 0,
};
const setFundTransModeModalVisible = ref<boolean>(false);
const setFundTransConfigModalVisible = ref<boolean>(false);
const setFundTransConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'custom',
  title: t('Td'),
  config: {} as KungfuApi.KfExtConfig,
});

const { searchKeyword, tableData } = useTableSearchKeyword<
  KungfuApi.KfConfig | KungfuApi.KfExtraLocation
>(td, ['group', 'name']);

const tableDataResolved = computed(() => {
  const tdGroupResolved: Record<string, KungfuApi.KfExtraLocation> = {};
  const tdResolved: KungfuApi.KfConfig[] = [];
  const markedNameToTdGroup: Record<string, KungfuApi.KfExtraLocation> = {};
  [...tdGroup.value, ...tableData.value].forEach((item) => {
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
const { getAssetMarginsByKfConfig, getAssetMarginsByTdGroup } =
  useAssetMargins();
const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
  useAddUpdateRemoveKfConfig();

const columns = computed(() => {
  const sorter = (dataIndex) => {
    return (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => {
      return (
        (+Number(getAssetsByKfConfig(a)[dataIndex as keyof KungfuApi.Asset]) ||
          0) -
        (+Number(getAssetsByKfConfig(b)[dataIndex as keyof KungfuApi.Asset]) ||
          0)
      );
    };
  };

  const marginSorter = (dataIndex) => {
    return (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => {
      return (
        (+Number(
          getAssetMarginsByKfConfig(a)[
            dataIndex as keyof KungfuApi.AssetMargin
          ],
        ) || 0) -
        (+Number(
          getAssetMarginsByKfConfig(b)[
            dataIndex as keyof KungfuApi.AssetMargin
          ],
        ) || 0)
      );
    };
  };

  if (currentGlobalKfLocation.value === null) {
    return getColumns(
      {
        category: 'td',
        group: '*',
        name: '*',
        mode: 'live',
      },
      sorter,
      marginSorter,
      isShowAssetMargin.value,
    );
  }

  return getColumns(
    currentGlobalKfLocation.value,
    sorter,
    marginSorter,
    isShowAssetMargin.value,
  );
});

const getPrefixByLocation = (kfLocation: KungfuApi.KfLocation) =>
  globalThis.HookKeeper.getHooks().prefix.trigger(kfLocation);

const { setTdGroups } = useGlobalStore();

onMounted(() => {
  setTdGroups().then(() => {
    tdGroupDataLoaded.value = true;
  });
});

async function handleOpenSetTdModal(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedSource: string,
  tdConfig?: KungfuApi.KfConfig,
) {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['td'] || {})[
    selectedSource
  ];

  if (!extConfig) {
    error(
      t('tdConfig.td_not_found', {
        td: selectedSource,
      }),
    );
    return;
  }

  currentSelectedSourceId.value = selectedSource;
  setTdConfigPayload.value.type = type;
  setTdConfigPayload.value.title = `${selectedSource} ${t('Td')}`;
  setTdConfigPayload.value.config =
    await globalThis.HookKeeper.getHooks().resolveExtConfig.trigger(
      {
        category: 'td',
        group: selectedSource,
        name: '*',
      },
      extConfig,
    );
  setTdConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (tdConfig) {
      setTdConfigPayload.value.initValue = JSON.parse(tdConfig.value);
    }
  }

  if (!extConfig?.settings?.length) {
    error(t('tdConfig.sourse_not_found'));
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
    name: t('tdConfig.td_group'),
    category: 'tdGroup',
    key: 'TdGroup',
    extPath: '',
    settings: [
      {
        key: 'td_group_name',
        name: t('tdConfig.td_name'),
        type: 'str',
        primary: true,
        required: true,
        tip: t('tdConfig.need_only_group'),
      },
    ],
  };
  addTdGroupConfigPayload.value.initValue = undefined;
  addTdGroupModalVisble.value = true;
}

function handleFundTransModeDialog(config: KungfuApi.KfConfig) {
  if (getProcessStatusName(config) !== 'Ready') return;
  currentAccout.source = config.group;
  currentAccout.config = config;
  currentAccout.avail = getAssetsByKfConfig(config).avail;
  setFundTransModeModalVisible.value = true;
}

function handleOpenSetFundTransModal(type: 'between_nodes' | 'tranc_in') {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['td'] || {})[
    currentAccout.source
  ];
  if (!extConfig.fund_trans) {
    error(
      t('fund_trans.config_error', {
        td: currentAccout.source,
      }),
    );
    return;
  }

  const selectFundTransConfig = extConfig.fund_trans[type];
  selectFundTransConfig.settings.forEach((item) => {
    if (item.key === 'amount' && type === 'between_nodes') {
      item.max = currentAccout.avail;
    }
  });
  currentAccout.transfer_type = type;
  setTdConfigPayload.value.initValue = undefined;
  setFundTransConfigPayload.value.title = t('fund_trans.modal_title');
  setFundTransConfigPayload.value.config = {
    type: [],
    name: t('fund_trans.modal_title'),
    category: 'td',
    key: currentAccout.source,
    extPath: '',
    settings: selectFundTransConfig.settings,
  };

  setFundTransConfigModalVisible.value = true;
}

function handleConfirmFundTrans(formState) {
  const watcher = window.watcher as KungfuApi.Watcher;
  const formStateResolved = {
    ...formState,
    key: getFundTransKey(currentAccout.transfer_type),
    update_time: '',
  };

  const message: KungfuApi.TimeKeyValue = {
    key: watcher.now().toString(),
    update_time: watcher.now(),
    value: JSON.stringify(formStateResolved),
    tag_a: getFundTransKey(null),
    tag_b: '',
    tag_c: '',
    source: 0,
    dest: 0,
    uid_key: '',
  };
  const fundTransResult = watcher.issueCustomData(
    message,
    currentAccout.config as KungfuApi.KfConfig,
  );

  if (
    formState.source &&
    formState.target &&
    formState.source === formState.target
  ) {
    error('划入节点和划出节点不能一致，请重新选择！');
    return;
  }

  if (fundTransResult) {
    success();
  } else {
    error(t('fund_trans.error'));
  }
}

function dealDisabledColor(config: KungfuApi.KfConfig) {
  return getProcessStatusName(config) === 'Ready'
    ? 'rgba(255, 255, 255, 1)'
    : 'rgba(255, 255, 255, 0.35)';
}

function handleConfirmAddUpdateTdGroup(
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  const { td_group_name } = formState;
  const newTdGroup: KungfuApi.KfExtraLocation = {
    category: 'tdGroup',
    group: 'group',
    name: td_group_name.toString(),
    mode: 'live',
    children: [],
  };

  return addTdGroup(newTdGroup)
    .then(() => {
      return setTdGroups();
    })
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
    });
}

function handleRemoveTdGroup(item: KungfuApi.KfExtraLocation) {
  confirmModal(
    t('tdConfig.delete_amount_group', {
      group: item.name,
    }),
    `${t('tdConfig.delete_amount_group', {
      group: item.name,
    })}, ${t('tdConfig.confirm_delete_group')}`,
  ).then((flag) => {
    if (!flag) return;
    removeTdGroup(item.name)
      .then(() => {
        return setTdGroups();
      })
      .then(() => {
        success();

        if (
          currentGlobalKfLocation.value &&
          getProcessIdByKfLocation(item) ===
            getProcessIdByKfLocation(currentGlobalKfLocation.value)
        ) {
          resetCurrentGlobalKfLocation();
        }
      })
      .catch((err) => {
        error(err.message || t('operation_failed'));
      });
  });
}

function handleRemoveTd(item: KungfuApi.KfConfig) {
  handleRemoveKfConfig(window.watcher, item, processStatusData.value)
    .then(() => {
      const accountId = getIdByKfLocation(item);
      const oldGroup = isInTdGroup(tdGroup.value, accountId);
      if (oldGroup) {
        const index = oldGroup.children?.indexOf(accountId);
        oldGroup.children.splice(index, 1);
        setTdGroup(toRaw(tdGroup.value)).then(() => {
          return setTdGroups();
        });
      }
    })
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
    });
}

function handleRequestPosition() {
  const res = window.watcher.requestPosition(window.watcher);
  if (res) {
    success(t('operation_success'));
  } else {
    error(t('operation_failed'));
  }
}
</script>

<template>
  <div class="kf-td__warp kf-translateZ">
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
          <a-switch
            :checked="allProcessOnline"
            @click="handleSwitchAllProcessStatus"
          ></a-switch>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleRequestPosition">
            <template #icon>
              <ReloadOutlined style="font-size: 14px" />
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleOpenAddTdGroupDialog('add')">
            {{ $t('tdConfig.add_group_placeholder') }}
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetSourceDialog"
          >
            {{ $t('tdConfig.add_td') }}
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
        :emptyText="$t('empty_text')"
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
            <div class="td-name__warp">
              <a-tag
                :color="getInstrumentTypeColor(tdExtTypeMap[record.group])"
              >
                {{ record.group }}
              </a-tag>
              <span>
                {{ record.name }}
              </span>
              <Icon
                v-if="getPrefixByLocation(record).prefixType === 'icon'"
                :component="getPrefixByLocation(record).prefix"
                style="font-size: 12px; margin-left: 7px"
              />
            </div>
          </template>
          <template
            v-else-if="
              column.dataIndex === 'name' && record.category === 'tdGroup'
            "
          >
            <div class="td-name__warp">
              <a-tag color="#FAAD14">{{ $t('tdConfig.account_group') }}</a-tag>
              <span>
                {{ record.name }}
              </span>
            </div>
          </template>
          <template
            v-else-if="
              column.dataIndex === 'accountName' && record.category === 'td'
            "
          >
            {{
              JSON.parse((record as KungfuApi.KfConfig).value).account_name ||
              '--'
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
              :loading="
                getIfProcessStopping(
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
              :num="
                dealAssetPrice(
                  getAssetsByKfConfig(record).margin ||
                    getAssetMarginsByKfConfig(record).margin,
                )
              "
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="
                dealAssetPrice(
                  getAssetsByTdGroup(record).margin ||
                    getAssetMarginsByTdGroup(record).margin,
                )
              "
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'avail'">
            <KfBlinkNum
              v-if="record.category === 'td'"
              mode="compare-zero"
              :num="dealAssetPrice(getAssetsByKfConfig(record).avail)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="dealAssetPrice(getAssetsByTdGroup(record).avail)"
            ></KfBlinkNum>
          </template>
          <template
            v-else-if="isShowAssetMargin && column.dataIndex === 'avail_margin'"
          >
            <KfBlinkNum
              v-if="record.category === 'td'"
              mode="compare-zero"
              :num="
                dealAssetPrice(getAssetMarginsByKfConfig(record).avail_margin)
              "
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="
                dealAssetPrice(getAssetMarginsByTdGroup(record).avail_margin)
              "
            ></KfBlinkNum>
          </template>
          <template
            v-else-if="isShowAssetMargin && column.dataIndex === 'cash_debt'"
          >
            <KfBlinkNum
              v-if="record.category === 'td'"
              mode="compare-zero"
              :num="dealAssetPrice(getAssetMarginsByKfConfig(record).cash_debt)"
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="record.category === 'tdGroup'"
              :num="dealAssetPrice(getAssetMarginsByTdGroup(record).cash_debt)"
            ></KfBlinkNum>
          </template>
          <template
            v-else-if="isShowAssetMargin && column.dataIndex === 'total_asset'"
          >
            <KfBlinkNum
              v-if="record.category === 'td'"
              mode="compare-zero"
              :num="
                dealAssetPrice(getAssetMarginsByKfConfig(record).total_asset)
              "
            ></KfBlinkNum>
            <KfBlinkNum
              v-else-if="isShowAssetMargin && record.category === 'tdGroup'"
              :num="
                dealAssetPrice(getAssetMarginsByTdGroup(record).total_asset)
              "
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div v-if="record.category === 'td'" class="kf-actions__warp">
              <PayCircleOutlined
                v-if="record.group === 'itp'"
                :style="{
                  color: dealDisabledColor(record as KungfuApi.KfConfig),
                }"
                @click.stop="
                  handleFundTransModeDialog(record as KungfuApi.KfConfig)
                "
              />
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogview(record)"
              />
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="
                  handleOpenSetTdModal(
                    'update',
                    record.group,
                    record as KungfuApi.KfConfig,
                  )
                "
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveTd(record as KungfuApi.KfConfig)"
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
    <KfSetExtensionModal
      v-if="setSourceModalVisible"
      v-model:visible="setSourceModalVisible"
      extension-type="td"
      @confirm="handleOpenSetTdModal('add', $event)"
    ></KfSetExtensionModal>
    <FundTransModal
      v-if="setFundTransModeModalVisible"
      v-model:visible="setFundTransModeModalVisible"
      @confirm="handleOpenSetFundTransModal($event)"
    ></FundTransModal>
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
    <KfSetByConfigModal
      v-if="setFundTransConfigModalVisible"
      v-model:visible="setFundTransConfigModalVisible"
      :payload="setFundTransConfigPayload"
      @confirm="({ formState }) => handleConfirmFundTrans(formState)"
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

  .td-name__warp {
    word-break: break-all;
  }
}
</style>
