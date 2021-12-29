<script setup lang="ts">
import { computed, reactive, ref } from 'vue';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfConfigSettingsForm from '@renderer/components/public/KfConfigSettingsForm.vue';
import { getIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    initFormStateByConfig,
    useCurrentGlobalKfLocation,
} from '@renderer/assets/methods/uiUtils';
import { getConfigSettings } from './config';

interface MakeOrderProps {}
defineProps<MakeOrderProps>();

const configSettings = computed(() => {
    if (!currentGlobalKfLocation.value) {
        return getConfigSettings();
    }

    const { category } = currentGlobalKfLocation.value;
    return getConfigSettings(category);
});

const formState = reactive(initFormStateByConfig(getConfigSettings(), {}));
const formRef = ref();

const { currentGlobalKfLocation, currentCategoryData } =
    useCurrentGlobalKfLocation(window.watcher);
</script>
<template>
    <div class="kf-make-order-dashboard__warp">
        <KfDashboard>
            <template v-slot:title>
                <span v-if="currentGlobalKfLocation.value">
                    <a-tag
                        v-if="currentCategoryData"
                        :color="currentCategoryData.color"
                    >
                        {{ currentCategoryData.name }}
                    </a-tag>
                    <span class="name" v-if="currentGlobalKfLocation.value">
                        {{
                            getIdByKfLocation(currentGlobalKfLocation.value) ||
                            ''
                        }}
                    </span>
                </span>
            </template>
            <template v-slot:header>
                <KfDashboardItem>
                    <a-button size="small">重置</a-button>
                </KfDashboardItem>
            </template>
            <div class="make-order__warp">
                <div class="make-order-form__warp">
                    <KfConfigSettingsForm
                        ref="formRef"
                        v-model:formState="formState"
                        :configSettings="configSettings"
                        changeType="add"
                        :label-col="4"
                        :wrapper-col="18"
                    ></KfConfigSettingsForm>
                </div>
                <div class="make-order-btns">
                    <a-button size="small">下单</a-button>
                </div>
            </div>
        </KfDashboard>
    </div>
</template>
<style lang="less">
.kf-make-order-dashboard__warp {
    width: 100%;
    height: 100%;

    .make-order__warp {
        display: flex;
        justify-content: space-between;
        height: 100%;

        .make-order-form__warp {
            flex: 1;
            height: 100%;
            padding-top: 16px;
            overflow-y: auto;
        }

        .make-order-btns {
            width: 40px;
            height: 100%;

            .ant-btn {
                height: 100%;
                text-align: center;
                word-break: break-word;
                word-wrap: unset;
                white-space: normal;
            }
        }
    }
}
</style>
