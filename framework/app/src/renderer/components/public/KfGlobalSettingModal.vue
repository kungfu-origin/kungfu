<script setup lang="ts">
import {
    getKfGlobalSettings,
    getKfGlobalSettingsValue,
    KfSystemConfig,
    setKfGlobalSettingsValue,
} from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
    initFormStateByConfig,
    useModalVisible,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import { onMounted, reactive, ref } from 'vue';
import KfConfigSettingsForm from './KfConfigSettingsForm.vue';
import {
    getKfCommission,
    setKfCommission,
} from '@kungfu-trader/kungfu-js-api/kungfu/commission';
import {
    CommissionMode,
    ExchangeIds,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { CommissionModeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { DeleteOutlined } from '@ant-design/icons-vue';
import { longfist } from '@kungfu-trader/kungfu-js-api/kungfu';

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
onMounted(() => {
    getKfCommission().then((res) => {
        commissions.value = res;
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

function handleCloseGlobalSetting() {
    setKfGlobalSettingsValue(globalSettingsFromStates);
    setKfCommission(commissions.value);
    closeModal();
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
        title="全局设置"
        :destoryOnClose="true"
        @cancel="handleCloseGlobalSetting"
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
                                :formState="
                                    globalSettingsFromStates[config.key]
                                "
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
                                <a-button
                                    type="primary"
                                    @click="handleAddCommission"
                                >
                                    添加
                                </a-button>
                            </div>
                            <div
                                class="commission-setting-row"
                                v-for="item in tableData"
                            >
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
                                            v-for="key in Object.keys(
                                                ExchangeIds,
                                            )"
                                            :key="key"
                                            :value="key"
                                        >
                                            {{ ExchangeIds[key].name }}
                                        </a-select-option>
                                    </a-select>
                                </div>
                                <div class="commission-setting-item">
                                    <a-select
                                        class="value"
                                        v-model:value="item.mode"
                                    >
                                        <a-select-option
                                            v-for="key in Object.keys(
                                                CommissionMode,
                                            )"
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
                                        @click="handleRemoveCommission(item)"
                                    />
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
                padding-left: 48px;

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
            }
        }

        .ant-tabs-content-holder {
            max-height: 680px;
            overflow: auto;
        }
    }
}
</style>
