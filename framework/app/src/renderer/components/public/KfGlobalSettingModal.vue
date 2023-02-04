<script setup lang="ts">
import {
  getKfGlobalSettings,
  KfSystemConfig,
  setKfGlobalSettingsValue,
} from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  useModalVisible,
  useWritableTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed, onMounted, onUnmounted, reactive, ref } from 'vue';
import KfConfigSettingsForm from './KfConfigSettingsForm.vue';
import {
  getKfCommission,
  setKfCommission,
} from '@kungfu-trader/kungfu-js-api/kungfu/commission';
import {
  CommissionMode,
  ExchangeIds,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { DeleteOutlined } from '@ant-design/icons-vue';
import { longfist } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  dealCommissionMode,
  initFormStateByConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useExtConfigsRelated } from '../../assets/methods/actionsUtils';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import { useGlobalStore } from '../../pages/index/store/global';
import { storeToRefs } from 'pinia';
import { useLanguage } from '@kungfu-trader/kungfu-js-api/language';

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

const store = useGlobalStore();
const { globalSetting } = storeToRefs(store);
const { uiExtConfigs } = useExtConfigsRelated();
const { isLanguageKeyAvailable } = useLanguage();

const globalSettingComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.value)
    .filter((key) => uiExtConfigs.value[key].position === 'global_setting')
    .map((key) => {
      return {
        ...uiExtConfigs.value[key],
        key,
      };
    });
});

const kfGlobalSettings = getKfGlobalSettings();
const globalSettingsFromStates = reactive(
  initGlobalSettingsFromStates(kfGlobalSettings, globalSetting.value),
);

const { modalVisible, closeModal } = useModalVisible(props.visible);
const commissions = ref<KungfuApi.Commission[]>([]);
const { searchKeyword, tableData } =
  useWritableTableSearchKeyword<KungfuApi.Commission>(commissions, [
    'product_id',
  ]);

onMounted(() => {
  globalBus.next({
    tag: 'open:globalSetting',
  });

  getKfCommission().then((res) => {
    commissions.value = res;
  });
});

onUnmounted(() => {
  globalBus.next({
    tag: 'close:globalSetting',
  });

  setKfGlobalSettingsValue(globalSettingsFromStates).then(() => {
    globalBus.next({
      tag: 'saved:globalSetting',
    });
    store.setKfGlobalSetting();
  });

  setKfCommission(commissions.value);
});

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
</script>
<template>
  <a-modal
    class="kf-global-settings-modal"
    width="1000px"
    v-model:visible="modalVisible"
    :title="$t('globalSettingConfig.global_setting_title')"
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
                v-model:formState="globalSettingsFromStates[config.key]"
                :configSettings="config.config"
                changeType="update"
                :primaryKeyAvoidRepeatCompareTarget="[]"
                primaryKeyAvoidRepeatCompareExtra=""
                layout="vertical"
              ></KfConfigSettingsForm>
            </a-tab-pane>
            <a-tab-pane
              key="comission"
              :tab="$t('globalSettingConfig.comission')"
            >
              <div class="search-input">
                <a-input-search
                  v-model:value="searchKeyword"
                  :placeholder="$t('globalSettingConfig.varieties')"
                  size="large"
                  style="width: 480px"
                />
                <a-button @click="handleAddCommission">
                  {{ $t('globalSettingConfig.add_comission') }}
                </a-button>
              </div>
              <div
                v-for="(item, index) in tableData"
                :key="index"
                class="commission-setting-row"
              >
                <div class="commission-setting-item">
                  <a-input
                    class="value product-id"
                    v-model:value="item.data.product_id"
                    :placeholder="$t('globalSettingConfig.varieties')"
                  ></a-input>
                </div>
                <div class="commission-setting-item">
                  <a-select
                    class="value exchange-id"
                    v-model:value="item.data.exchange_id"
                    :placeholder="$t('globalSettingConfig.exchange_id')"
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
                  <a-select class="value" v-model:value="item.data.mode">
                    <a-select-option
                      v-for="key in Object.keys(CommissionMode)"
                      :key="key"
                      :value="+key"
                    >
                      {{ dealCommissionMode(+key).name }}
                    </a-select-option>
                  </a-select>
                </div>
                <div class="commission-setting-item">
                  <span class="label">
                    {{ $t('globalSettingConfig.open') }}
                  </span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.data.open_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">
                    {{ $t('globalSettingConfig.close_yesterday') }}
                  </span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.data.close_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">
                    {{ $t('globalSettingConfig.close_today') }}
                  </span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.data.close_today_ratio"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <span class="label">{{ $t('globalSettingConfig.min') }}</span>
                  <a-input-number
                    class="value"
                    :precision="8"
                    step="0.00000001"
                    v-model:value="item.data.min_commission"
                  ></a-input-number>
                </div>
                <div class="commission-setting-item">
                  <DeleteOutlined
                    class="kf-hover"
                    style="font-size: 14px"
                    @click="handleRemoveCommission(item.data)"
                  />
                </div>
              </div>
            </a-tab-pane>
            <a-tab-pane
              v-for="config in globalSettingComponentConfigs"
              :key="config.key"
              :tab="
                isLanguageKeyAvailable(config.name)
                  ? $t(config.name)
                  : config.name
              "
            >
              <component :is="config.key"></component>
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
