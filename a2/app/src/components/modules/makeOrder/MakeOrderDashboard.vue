<script setup lang="ts">
import { computed, reactive, ref, watch } from 'vue';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfConfigSettingsForm from '@renderer/components/public/KfConfigSettingsForm.vue';
import {
    getIdByKfLocation,
    getMdTdKfLocationByProcessId,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    initFormStateByConfig,
    useCurrentGlobalKfLocation,
    useExtConfigsRelated,
    useInstruments,
    useProcessStatusDetailData,
    useTriggeMakeOrder,
} from '@renderer/assets/methods/uiUtils';
import { getConfigSettings } from './config';
import { message } from 'ant-design-vue';
import { kfMakeOrder } from '@kungfu-trader/kungfu-js-api/kungfu';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';

interface MakeOrderProps {}
defineProps<MakeOrderProps>();

const makeOrderInstrumentType = ref<InstrumentTypeEnum>(
    InstrumentTypeEnum.unknown,
);
const configSettings = computed(() => {
    if (!currentGlobalKfLocation.value) {
        return getConfigSettings();
    }

    const { category } = currentGlobalKfLocation.value;
    return getConfigSettings(category, makeOrderInstrumentType.value);
});

const formState = reactive(initFormStateByConfig(getConfigSettings(), {}));
const formRef = ref();
const {
    subscribeAllInstrumentByAppStates,
    transformSearchInstrumentResultToInstrument,
} = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap } = useExtConfigsRelated();
const { triggeOrderBook } = useTriggeMakeOrder();

const { currentGlobalKfLocation, currentCategoryData } =
    useCurrentGlobalKfLocation(window.watcher);

watch(
    () => formState.instrument,
    (newVal) => {
        const instrumentResolved = transformSearchInstrumentResultToInstrument(
            newVal.toString(),
        );

        if (!instrumentResolved) {
            return;
        }

        subscribeAllInstrumentByAppStates(
            processStatusData.value,
            appStates.value,
            mdExtTypeMap.value,
            [instrumentResolved],
        );

        triggeOrderBook(instrumentResolved);
        makeOrderInstrumentType.value = instrumentResolved.instrumentType;
    },
);

function handleMakeOrder() {
    formRef.value
        .validate()
        .then(() => {
            const instrument = formState.instrument.toString();
            const instrumnetResolved =
                transformSearchInstrumentResultToInstrument(instrument);

            if (!instrumnetResolved) {
                message.error('标的信息错误');
                return;
            }

            const { exchangeId, instrumentId, instrumentType } =
                instrumnetResolved;

            const {
                account_id,
                limit_price,
                volume,
                price_type,
                side,
                offset,
                hedge_flag,
            } = formState;

            const makeOrderInput: KungfuApi.MakeOrderInput = {
                instrument_id: instrumentId,
                instrument_type: +instrumentType,
                exchange_id: exchangeId,
                limit_price: +limit_price,
                volume: +volume,
                price_type: +price_type,
                side: +side,
                offset: +(offset !== undefined ? offset : +side === 0 ? 0 : 1),
                hedge_flag: +(hedge_flag || 0),
            };

            if (!currentGlobalKfLocation.value) {
                message.error('当前 Location 错误');
                return;
            }

            if (currentGlobalKfLocation.value.category === 'td') {
                kfMakeOrder(
                    window.watcher,
                    makeOrderInput,
                    currentGlobalKfLocation.value,
                ).catch((err) => {
                    message.error(err);
                });
            } else if (currentGlobalKfLocation.value.category === 'strategy') {
                const tdLocation = getMdTdKfLocationByProcessId(
                    `td_${account_id}`,
                );
                if (!tdLocation) {
                    message.error('下单账户信息错误');
                    return;
                }
                kfMakeOrder(
                    window.watcher,
                    makeOrderInput,
                    tdLocation,
                    currentGlobalKfLocation.value,
                ).catch((err) => {
                    message.error(err);
                });
            } else {
                message.error('当前 Location 类型错误');
            }
        })
        .catch((err: Error) => {
            console.error(err);
        });
}
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
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleMakeOrder"
                    >
                        下单
                    </a-button>
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

            .ant-form-item {
                margin-bottom: 16px;

                .ant-form-item-explain,
                .ant-form-item-extra {
                    min-height: unset;
                }
            }
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
