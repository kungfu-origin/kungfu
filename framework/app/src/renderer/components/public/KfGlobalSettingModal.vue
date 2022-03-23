<script setup lang="ts">
import {
  getKfGlobalSettings,
  getKfGlobalSettingsValue,
  KfSystemConfig,
  setKfGlobalSettingsValue,
} from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  useAllKfConfigData,
  useModalVisible,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  computed,
  ComputedRef,
  onMounted,
  onUnmounted,
  reactive,
  ref,
  toRefs,
} from 'vue';
import KfConfigSettingsForm from './KfConfigSettingsForm.vue';
import {
  getKfCommission,
  setKfCommission,
} from '@kungfu-trader/kungfu-js-api/kungfu/commission';
import {
  CommissionMode,
  ExchangeIds,
  KfCategory,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  CommissionModeEnum,
  KfCategoryEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { DeleteOutlined } from '@ant-design/icons-vue';
import { longfist } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  getScheduleTasks,
  setScheduleTasks,
} from '@kungfu-trader/kungfu-js-api/actions';
import {
  getProcessIdByKfLocation,
  initFormStateByConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import dayjs, { Dayjs } from 'dayjs';
import {
  coreForScheduleTasksOptions,
  modeForCoreScheduleTasksOptions,
  modeForScheduleTasksOptions,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/configs';
import { ipcRenderer } from 'electron';

interface ScheduleTaskFormItem {
  timeValue: Dayjs;
  mode: KungfuApi.ScheduleTaskMode;
  processId: string;
}

const props = withDefaults(
  defineProps<{
    visible: boolean;
  }>(),
  {
    visible: false,
  },
);

defineEmits<{
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const kfGlobalSettings = getKfGlobalSettings();
const kfGlobalSettingsValue = getKfGlobalSettingsValue();
const { modalVisible, closeModal } = useModalVisible(props.visible);
const commissions = ref<KungfuApi.Commission[]>([]);
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.Commission>(commissions, ['product_id']);

const { td, md, strategy } = toRefs(useAllKfConfigData());

const kfConfigForScheduleTasksOptions: ComputedRef<
  Array<{ label: string; value: string; category: string }>
> = computed(() => {
  return [...td.value, ...md.value, ...strategy.value].map((item) => ({
    label: getProcessIdByKfLocation(item),
    value: getProcessIdByKfLocation(item),
    category: item.category,
  }));
});

const scheduleTask = reactive<{
  active?: boolean;
  tasks?: ScheduleTaskFormItem[];
}>({});

onMounted(() => {
  getKfCommission().then((res) => {
    commissions.value = res;
  });

  getScheduleTasks().then((res) => {
    scheduleTask.active = !!res.active;
    scheduleTask.tasks = !res.tasks
      ? [
          {
            processId: 'core',
            mode: 'restart',
            timeValue: dayjs('08:00:00', 'HH:mm:ss'),
          },
          {
            processId: 'core',
            mode: 'restart',
            timeValue: dayjs('20:00:00', 'HH:mm:ss'),
          },
        ]
      : (res.tasks || [])
          .sort((item1, item2) =>
            item1.processId.localeCompare(item2.processId),
          )
          .map((item) => {
            return {
              ...item,
              timeValue: dayjs(
                `${item.hour}:${item.minute}:${item.second}`,
                'HH:mm:ss',
              ),
            };
          });
  });
});

onUnmounted(() => {
  setKfGlobalSettingsValue(globalSettingsFromStates);
  setKfCommission(commissions.value);
  setScheduleTasks({
    active: scheduleTask.active || false,
    tasks: (scheduleTask.tasks || [])
      .filter((item) => !!item.processId)
      .map((item) => ({
        hour: dayjs(item.timeValue, 'HH:mm:ss').hour().toString(),
        minute: dayjs(item.timeValue, 'HH:mm:ss').minute().toString(),
        second: dayjs(item.timeValue, 'HH:mm:ss').second().toString(),
        mode: item.mode,
        processId: item.processId,
      })),
  }).then(() => {
    ipcRenderer.send('schedule-setting-refresh');
  });
});

const globalSettingsFromStates = reactive(
  initGlobalSettingsFromStates(kfGlobalSettings, kfGlobalSettingsValue),
);

function initGlobalSettingsFromStates(
  configs: KfSystemConfig[],
  initValue: Record<string, Record<string, KungfuApi.KfConfigValue>>,
) {
  const states: Record<string, Record<string, KungfuApi.KfConfigValue>> = {};
  configs.forEach((item) => {
    states[item.key] = initFormStateByConfig(
      item.config,
      initValue[item.key] || {},
    );
  });
  return states;
}

function handleRemoveCommission(commission: KungfuApi.Commission) {
  const index = commissions.value.findIndex((item) => {
    if (item.product_id === commission.product_id) {
      if (item.exchange_id === commission.exchange_id) {
        return true;
      }
    }

    return false;
  });

  if (index !== -1) {
    commissions.value.splice(index, 1);
  }
}

function handleAddCommission() {
  const newCommission = longfist.Commission();
  commissions.value.unshift(newCommission);
}

function handleAddScheduleTask() {
  if (scheduleTask.tasks) {
    scheduleTask.tasks.push({
      processId: '',
      mode: 'start',
      timeValue: dayjs(),
    });
  }
}

function handleRemoveScheduleTask(index: number) {
  if (scheduleTask.tasks) {
    scheduleTask.tasks.splice(index, 1);
  }
}
</script>
<template>
  <a-modal
    class="kf-global-settings-modal"
    width="1000px"
    v-model:visible="modalVisible"
    title="全局设置"
    :destoryOnClose="true"
    @cancel="closeModal"
    :footer="null"
  >
    <div class="kf-global-settings-modal__content">
      <div class="body-content">
        <div class="config-settings-nav__content"></div>
        <div class="config-settings__content">
          <a-tabs
            type="card"
            tab-position="left"
            :tabBarStyle="{ 'max-height': '680px' }"
          >
            <a-tab-pane
              v-for="config in kfGlobalSettings"
              :key="config.key"
              :tab="config.name"
            >
              <KfConfigSettingsForm
                :formState="globalSettingsFromStates[config.key]"
                :configSettings="config.config"
                changeType="update"
                :primaryKeyAvoidRepeatCompareTarget="[]"
                primaryKeyAvoidRepeatCompareExtra=""
                layout="vertical"
              ></KfConfigSettingsForm>
            </a-tab-pane>
            <a-tab-pane key="comission" tab="期货手续费">
              <div class="search-input">
                <a-input-search
                  v-model:value="searchKeyword"
                  placeholder="品种"
                  size="large"
                  style="width: 480px"
                />
                <a-button @click="handleAddCommission">添加</a-button>
              </div>
              <div class="commission-setting-row" v-for="item in tableData">
                <div class="commission-setting-item">
                  <a-input
                    class="value product-id"
                    v-model:value="item.product_id"
                    placeholder="品种"
                  ></a-input>
                </div>
                <div class="commission-setting-item">
                  <a-select
                    class="value exchange-id"
                    v-model:value="item.exchange_id"
                    placeholder="交易所"
                  >
                    <a-select-option
                      v-for="key in Object.keys(ExchangeIds)"
                      :key="key"
                      :value="key"
                    >
                      {{ ExchangeIds[key].name }}
                    </a-select-option>
                  </a-select>
                </div>
                <div class="commission-setting-item">
                  <a-select class="value" v-model:value="item.mode">
                    <a-select-option
                      v-for="key in Object.keys(CommissionMode)"
                      :key="key"
                      :value="+key"
                    >
                      {{ CommissionMode[+key as CommissionModeEnum].name }}
                    </a-select-option>
                  </a-select>
                </div>
                <div class="commission-setting-item">
                  <span class="label">开仓</span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.open_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">平昨</span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.close_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">平今</span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.close_today_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">最小</span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.min_commission"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <delete-outlined
                    class="kf-hover"
                    style="font-size: 14px"
                    @click="handleRemoveCommission(item)"
                  />
                </div>
              </div>
            </a-tab-pane>
            <a-tab-pane key="schedule" tab="定时起停">
              <div class="global-setting-item">
                <div class="label" title="使用定时起停">使用定时起停</div>
                <div class="value">
                  <a-switch
                    size="small"
                    v-model:checked="scheduleTask.active"
                  ></a-switch>
                </div>
              </div>
              <div class="global-setting-item">
                <div class="label">定时起停任务列表</div>
                <a-button
                  style="margin-bottom: 16px"
                  @click="handleAddScheduleTask"
                >
                  添加定时
                </a-button>
                <div
                  class="value schedule-setting__warp"
                  v-for="(task, index) in scheduleTask.tasks || []"
                >
                  <a-row>
                    <a-col>
                      <div class="title">目标进程</div>
                      <a-select
                        style="width: 220px"
                        v-model:value="task.processId"
                        :disabled="task.processId === 'core'"
                        option-label-prop="label"
                      >
                        <a-select-option
                          v-for="item in task.processId === 'core'
                            ? coreForScheduleTasksOptions
                            : kfConfigForScheduleTasksOptions"
                          :value="item.value"
                          :label="item.label"
                        >
                          <a-tag
                            :color="
                              KfCategory[KfCategoryEnum[item.category]].color
                            "
                          >
                            {{ KfCategory[KfCategoryEnum[item.category]].name }}
                          </a-tag>
                          {{ item.label }}
                        </a-select-option>
                      </a-select>
                    </a-col>
                    <a-col>
                      <div class="title">方式</div>
                      <a-select
                        style="width: 120px"
                        v-model:value="task.mode"
                        :disabled="task.processId === 'core'"
                      >
                        <a-select-option
                          v-for="item in task.processId === 'core'
                            ? modeForCoreScheduleTasksOptions
                            : modeForScheduleTasksOptions"
                          :value="item.value"
                        >
                          {{ item.label }}
                        </a-select-option>
                      </a-select>
                    </a-col>
                    <a-col>
                      <div class="title">每日时间</div>
                      <a-time-picker
                        style="width: 120px"
                        v-model:value="task.timeValue"
                      ></a-time-picker>
                    </a-col>
                    <a-col v-if="task.processId !== 'core'">
                      <div class="title"></div>
                      <delete-outlined
                        class="kf-hover"
                        style="font-size: 14px"
                        @click="handleRemoveScheduleTask(index)"
                      />
                    </a-col>
                  </a-row>
                </div>
              </div>
            </a-tab-pane>
          </a-tabs>
        </div>
      </div>
    </div>
  </a-modal>
</template>

<style lang="less">
.kf-global-settings-modal__content {
  min-height: 370px;

  .body-content {
    .ant-tabs-content {
      .ant-tabs-tabpane {
        padding-left: 28px;
        box-sizing: border-box;

        .search-input {
          display: flex;
          text-align: center;
          margin-bottom: 24px;
          justify-content: center;
          align-items: center;

          .ant-input-group-wrapper.ant-input-search {
            margin-right: 30px;
          }
        }

        .ant-form-item-label > label,
        .global-setting-item .label {
          font-size: 14px;
        }

        .commission-setting-row {
          display: flex;
          justify-content: space-around;
          margin-bottom: 16px;

          .commission-setting-item {
            flex: 1;
            padding-right: 16px;
            box-sizing: border-box;
            display: flex;
            justify-content: space-between;
            align-items: center;

            .label {
              padding-right: 8px;
            }

            .value {
              &.product-id {
                width: 80px;
              }

              &.exchange-id {
                width: 80px;
              }
            }
          }
        }

        .global-setting-item {
          margin-bottom: 28px;

          .label {
            padding-bottom: 8px;
            font-size: 18px;
          }

          .value {
            &.schedule-setting__warp {
              .ant-row {
                margin-bottom: 10px;

                .ant-col {
                  display: flex;
                  align-items: center;
                  margin-right: 20px;
                }
              }

              .title {
                font-size: 12px;
                color: rgba(255, 255, 255, 0.45);
                margin-bottom: 4px;
                margin-right: 8px;
                padding-left: 11px;
              }
            }
          }
        }
      }
    }

    .ant-tabs-content-holder {
      max-height: 680px;
      overflow-y: overlay;
    }
  }
}
</style>
