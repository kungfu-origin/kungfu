<script lang="ts" setup>
import Icon, { ClusterOutlined, FileTextOutlined } from '@ant-design/icons-vue';
import { notification } from 'ant-design-vue';

import KfProcessStatus from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfProcessStatus.vue';

import {
  computed,
  ref,
  watch,
  onMounted,
  onBeforeUnmount,
  getCurrentInstance,
} from 'vue';
import { SystemProcessName } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  getInstrumentTypeColor,
  handleOpenLogview,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getKfCategoryData,
  getIfProcessRunning,
  getProcessIdByKfLocation,
  getPropertyFromProcessStatusDetailDataByKfLocation,
  getIfProcessStopping,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  handleSwitchProcessStatusGenerator,
  useAllKfConfigData,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;

const app = getCurrentInstance();
const handleSwitchProcessStatus = handleSwitchProcessStatusGenerator();
const processControllerBoardVisible = ref<boolean>(false);
const categoryList: (KfCategoryTypes | string)[] = [
  'system',
  'daemon',
  'td',
  'md',
  'strategy',
];
const allKfConfigData = useAllKfConfigData();
const {
  appStates,
  processStatusData,
  processStatusDetailData,
  getProcessStatusName,
} = useProcessStatusDetailData();
const { tdExtTypeMap, mdExtTypeMap } = useExtConfigsRelated();

let isClosingWindow = false;
let isRestartSystem = 0;
let hasAlertMasterStop = false;
let hasAlertLedgerStop = false;
let hasAlertCacheDStop = false;

const getNotificationType = (flag: number) => {
  return flag ? 'warning' : 'error';
};

watch(processStatusData, (newPSD, oldPSD) => {
  if (isClosingWindow) return;

  if (newPSD.master !== 'online' && oldPSD.master === 'online') {
    if (isRestartSystem || !hasAlertMasterStop) {
      hasAlertMasterStop = true;
      notification[getNotificationType(isRestartSystem)]({
        message: t('master_interrupt'),
        description: t('master_desc'),
        duration: 8,
        placement: 'bottomRight',
      });
      isRestartSystem && isRestartSystem++;
    }
  }

  if (newPSD.cached !== 'online' && oldPSD.cached === 'online') {
    if (isRestartSystem || !hasAlertCacheDStop) {
      hasAlertCacheDStop = true;
      notification[getNotificationType(isRestartSystem)]({
        message: t('cached_interrupt'),
        description: t('cached_desc'),
        duration: 8,
        placement: 'bottomRight',
      });
      isRestartSystem && isRestartSystem++;
    }
  }

  if (newPSD.ledger !== 'online' && oldPSD.ledger === 'online') {
    if (isRestartSystem || !hasAlertLedgerStop) {
      hasAlertLedgerStop = true;
      notification[getNotificationType(isRestartSystem)]({
        message: t('ledger_interrupt'),
        description: t('ledger_desc'),
        duration: 8,
        placement: 'bottomRight',
      });
      isRestartSystem && isRestartSystem++;
    }
  }

  if (isRestartSystem >= 4) {
    isRestartSystem = 0;
  }
});

watch(appStates, (newAppStates, oldAppStates) => {
  Object.keys(newAppStates || {}).forEach((key: string) => {
    const newState = newAppStates[key];
    const oldState = oldAppStates[key];

    if (
      newState === 'DisConnected' &&
      oldState !== 'DisConnected' &&
      processStatusData.value[key] === 'online'
    ) {
      notification.warning({
        message: t('state_interrupt_msg', {
          state: key,
        }),
        description: t('state_interrupt_desc', {
          state: key,
        }),
        duration: 8,
        placement: 'bottomRight',
      });
    }
  });
});

const mainStatusWell = computed(() => {
  const masterIsLive = processStatusData.value['master'] === 'online';
  const ledgerIsLive = processStatusData.value['ledger'] === 'online';
  const cachedIsLive = processStatusData.value['cached'] === 'online';
  return masterIsLive && ledgerIsLive && cachedIsLive;
});

function handleOpenProcessControllerBoard(): void {
  processControllerBoardVisible.value = true;
}

const prefixMap = ref({});

watch(
  () => allKfConfigData,
  () => {
    prefixMap.value = categoryList.reduce((map, category) => {
      allKfConfigData[category].forEach((location) => {
        map[getProcessIdByKfLocation(location)] =
          globalThis.HookKeeper.getHooks().prefix.trigger(location);
      });
      return map;
    }, {});
  },
  { deep: true },
);

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$globalBus.subscribe((data) => {
      if (data.tag === 'main') {
        if (data.name === 'clear-process-before-quit-start') {
          isClosingWindow = true;
        }
      }
      if (data.tag === 'processStatus') {
        if (data.name === 'system' && data.status === 'waiting restart') {
          !isRestartSystem && (isRestartSystem = 1);
        }
      }
    });
    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});
</script>

<template>
  <div
    :class="{
      'kf-process-status-controller__warp': true,
      'some-process-error': !mainStatusWell,
    }"
    @click="handleOpenProcessControllerBoard"
  >
    <ClusterOutlined style="font-size: 14px; padding-right: 4px" />
    <span class="title">{{ $t('baseConfig.control_center') }}</span>

    <a-drawer
      v-model:visible="processControllerBoardVisible"
      :width="650"
      class="kf-process-status-controller-board__warp"
      :title="$t('baseConfig.control_center')"
      placement="right"
    >
      <div
        class="process-controller-item"
        v-for="category in categoryList"
        :key="category"
      >
        <template v-if="allKfConfigData[category].length">
          <div class="kf-config-list">
            <div
              class="kf-config-item"
              v-for="config in allKfConfigData[category]"
            >
              <div class="process-info">
                <div class="category info-item">
                  <a-tag :color="getKfCategoryData(config.category).color">
                    {{ getKfCategoryData(config.category).name }}
                  </a-tag>
                </div>
                <div
                  class="process-id info-item"
                  v-if="config.category === 'system'"
                >
                  {{ (SystemProcessName[config.name] || {}).name || '' }}
                </div>
                <div
                  class="process-id info-item"
                  v-else-if="config.category !== 'strategy'"
                >
                  <a-tag
                    v-if="config.category === 'td' || config.category === 'md'"
                    :color="
                      getInstrumentTypeColor(
                        tdExtTypeMap[config.group] ||
                          mdExtTypeMap[config.group],
                      )
                    "
                  >
                    {{ config.group }}
                  </a-tag>
                  {{ config.name }}
                </div>
                <div class="process-id info-item" v-else>
                  {{ config.name }}
                </div>
                <Icon
                  v-if="
                    prefixMap[getProcessIdByKfLocation(config)]?.prefixType ===
                    'icon'
                  "
                  :component="
                    prefixMap[getProcessIdByKfLocation(config)].prefix
                  "
                  style="font-size: 12px"
                />
              </div>
              <div class="state-status">
                <KfProcessStatus
                  :statusName="getProcessStatusName(config)"
                ></KfProcessStatus>
              </div>
              <div class="switch">
                <a-switch
                  size="small"
                  :checked="
                    getIfProcessRunning(
                      processStatusData,
                      getProcessIdByKfLocation(config),
                    )
                  "
                  :loading="
                    getIfProcessStopping(
                      processStatusData,
                      getProcessIdByKfLocation(config),
                    )
                  "
                  @click="
                                    (checked: boolean, Event: MouseEvent) => 
                                        handleSwitchProcessStatus(
                                            checked,
                                            Event,
                                            config,
                                        )
                                    "
                ></a-switch>
              </div>
              <div class="cpu">
                CPU:
                {{
                  getPropertyFromProcessStatusDetailDataByKfLocation(
                    processStatusDetailData,
                    config,
                  ).cpu + '%'
                }}
              </div>
              <div class="memory">
                MEM:
                {{
                  getPropertyFromProcessStatusDetailDataByKfLocation(
                    processStatusDetailData,
                    config,
                  ).memory + 'M'
                }}
              </div>
              <div class="actions kf-actions__warp">
                <FileTextOutlined
                  @click="handleOpenLogview(config)"
                  style="font-size: 14px"
                ></FileTextOutlined>
              </div>
            </div>
          </div>
        </template>
      </div>
    </a-drawer>
  </div>
</template>

<style lang="less">
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/variables.less';

.kf-process-status-controller__warp {
  display: flex;
  align-items: center;

  &.some-process-error {
    .title {
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .anticon {
      color: lighten(@red2-base, 10%);
    }
  }
}

.kf-process-status-controller-board__warp {
  .process-controller-item {
    margin-bottom: 24px;

    .category-title {
      font-size: 18px;
      font-weight: bold;
      padding-bottom: 8px;
    }

    .kf-config-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 8px;

      .process-info {
        flex: 1;
        display: flex;
        justify-content: flex-start;
        align-items: center;
        margin-right: 8px;

        .info-item {
          margin-right: 8px;

          &.category {
            width: 70px;
          }
        }
      }

      .state-status {
        width: 80px;
      }

      .switch {
        width: 40px;
      }

      .cpu {
        width: 80px;
      }

      .memory {
        width: 120px;
      }

      .actions {
        width: 40px;
      }
    }
  }
}
</style>
