<script lang="ts" setup>
import { ClusterOutlined, FileTextOutlined } from '@ant-design/icons-vue';
import { notification } from 'ant-design-vue';

import KfProcessStatus from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfProcessStatus.vue';

import { computed, ref, watch } from 'vue';
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
  handleSwitchProcessStatus,
  useAllKfConfigData,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';

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

let hasAlertMasterStop = false;
let hasAlertLedgerStop = false;
let hasAlertCacheDStop = false;

watch(processStatusData, (newPSD, oldPSD) => {
  if (newPSD.master !== 'online' && oldPSD.master === 'online') {
    if (!hasAlertMasterStop) {
      hasAlertMasterStop = true;
      notification.error({
        message: '主控进程 master 中断',
        description: '主控进程负责策略进程间通信与资源配置, 请重启功夫交易系统',
        duration: 8,
        placement: 'bottomRight',
      });
    }
  }

  if (newPSD.ledger !== 'online' && oldPSD.ledger === 'online') {
    if (!hasAlertLedgerStop) {
      hasAlertLedgerStop = true;
      notification.error({
        message: '计算服务 ledger 中断',
        description: '计算服务负责持仓跟资金计算, 请重启功夫交易系统',
        duration: 8,
        placement: 'bottomRight',
      });
    }
  }

  if (newPSD.cached !== 'online' && oldPSD.cached === 'online') {
    if (!hasAlertCacheDStop) {
      hasAlertCacheDStop = true;
      notification.error({
        message: '存储服务 cached 中断',
        description:
          '存储服务负责数据落地, 存储服务断开不影响交易, 可等交易完成后重启功夫系统',
        duration: 8,
        placement: 'bottomRight',
      });
    }
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
        message: `${key} 已断开`,
        description: `${key} 已断开, 可能会导致交易中断, 请检查`,
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
    <span class="title">控制中心</span>

    <a-drawer
      v-model:visible="processControllerBoardVisible"
      :width="650"
      class="kf-process-status-controller-board__warp"
      title="控制中心"
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
  float: right;
  height: 100%;
  padding: 0 8px;
  display: flex;
  align-items: center;
  cursor: pointer;

  &.some-process-error {
    .title {
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .anticon {
      color: lighten(@red2-base, 10%);
    }
  }

  &:hover {
    background: @item-active-bg;
    color: @primary-color;
  }

  .title {
    font-size: 12px;
    font-weight: bold;
    color: @primary-color;
  }

  .anticon {
    color: @primary-color;
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
