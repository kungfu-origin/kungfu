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
} from '@renderer/assets/methods/uiUtils';
import { reactive } from 'vue';
import KfConfigSettingsForm from './KfConfigSettingsForm.vue';

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
    closeModal();
}
</script>
<template>
    <a-modal
        class="kf-global-settings-modal"
        width="800px"
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
                    <a-tabs type="card" tab-position="left">
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
                    </a-tabs>
                </div>
            </div>
        </div>
    </a-modal>
</template>

<style lang="less">
.kf-global-settings-modal__content {
    .body-content {
        .ant-tabs-content {
            .ant-tabs-tabpane {
                padding-left: 48px;
            }
        }
    }
}
</style>
