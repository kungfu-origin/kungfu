<template>
    <a-modal
        class="kf-set-by-config-modal"
        v-model:visible="modalVisible"
        :title="titleResolved"
        @cancel="handleClose"
        @ok="handleConfirm"
    >
        <a-form
            :form="formData"
            :label-col="{ span: 6 }"
            :wrapper-col="{ span: 16 }"
        >
            <a-form-item
                v-for="item in config?.settings"
                :key="item.key"
                :label="item.name"
            >
                <a-input v-if="item.type === 'str'"></a-input>
                <a-input-password
                    v-if="item.type === 'password'"
                ></a-input-password>
                <a-input-number
                    v-if="item.type === 'int'"
                    :precision="0"
                    step="1"
                ></a-input-number>
                <a-input-number
                    v-if="item.type === 'float'"
                    :precision="4"
                    step="0.0001"
                ></a-input-number>
                <a-input-number
                    v-if="item.type === 'percent'"
                    :precision="2"
                    step="0.01"
                    :formatter="(value: number) => `${value}%`"
                    :parser="(value: string) => value.replace('%', '')"
                ></a-input-number>
                <a-select v-if="inputTypeToTradeConfig[item.type]">
                    {{ item.type }}
                    <a-select-option
                        v-for="key in Object.keys(
                            inputTypeToTradeConfig[item.type],
                        )"
                        :key="key"
                        :value="key"
                    >
                        {{
                            getKfTradeValueName(
                                inputTypeToTradeConfig[item.type],
                                key,
                            )
                        }}
                    </a-select-option>
                </a-select>
                <a-select v-if="item.type === 'select'">
                    <a-select-option
                        v-for="option in item.options"
                        :key="option.value"
                        :value="option.value"
                    >
                        {{ option.label }}
                    </a-select-option>
                </a-select>

                <div
                    v-if="item.type === 'file'"
                    class="kf-form-item__warp file"
                >
                    <span
                        class="file-path"
                        v-if="formData[item.key]"
                        :title="formData[item.key] || ''"
                    >
                        {{ formData[item.key] }}
                    </span>
                    <a-button size="mini" @click="handleSelectFile(item.key)">
                        <template #icon><DashOutlined /></template>
                    </a-button>
                </div>
            </a-form-item>
        </a-form>
    </a-modal>
</template>

<script lang="ts">
import { defineComponent, PropType, ref } from 'vue';
import { DashOutlined } from '@ant-design/icons-vue';

import { modalVisibleComposition } from '@renderer/assets/methods/uiUtils';
import {
    KfConfigValue,
    KfTradeValueCommonData,
    SetKfConfigPayload,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    CommissionMode,
    Direction,
    ExchangeIds,
    HedgeFlag,
    InstrumentType,
    Offset,
    PriceType,
    Side,
    TimeCondition,
    VolumeCondition,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { dialog } from '@electron/remote';

export default defineComponent({
    props: {
        visible: {
            type: Boolean as PropType<boolean>,
            default: false,
        },

        payload: {
            type: Object as PropType<SetKfConfigPayload>,
            default: () => ({} as SetKfConfigPayload),
        },
    },

    components: {
        DashOutlined,
    },

    setup(props, context) {
        const { modalVisible, closeModal } = modalVisibleComposition(
            props,
            context,
        );

        const initFormDataByConfig = (
            config: SetKfConfigPayload['config'],
            initValue?: Record<string, KfConfigValue>,
        ): Record<string, KfConfigValue> => {
            const settings = config?.settings;
            if (!settings) return {};

            const booleanType = ['bool'];
            const numberType = [
                'int',
                'float',
                'percent',
                'side', // select - number
                'offset', // select - number
                'direction', // select - number
                'priceType', // select - number
                'hedgeFlag', // select - number
                'volumeCondition', // select - number
                'timeCondition', // select - number
                'commissionMode', // select - number
                'instrumentType', // select - number
            ];
            const formData: Record<string, KfConfigValue> = {};
            settings.forEach((item) => {
                const type = item.type;
                const isBoolean = booleanType.includes(type);
                const isNumber = numberType.includes(type);

                let defaultValue = item?.default;
                if (defaultValue === undefined) {
                    defaultValue = isBoolean ? false : isNumber ? 0 : '';
                }
                if ((initValue || {})[item.key] !== undefined) {
                    defaultValue = (initValue || {})[item.key];
                }

                formData[item.key] = defaultValue;
            });

            return formData;
        };

        const inputTypeToTradeConfig: {
            [prop: string]: Record<
                string | number | symbol,
                KfTradeValueCommonData
            >;
        } = {
            exchange: ExchangeIds,
            side: Side,
            offset: Offset,
            direction: Direction,
            priceType: PriceType,
            hedgeFlag: HedgeFlag,
            volumeCondition: VolumeCondition,
            timeCondition: TimeCondition,
            commissionMode: CommissionMode,
            instrumentType: InstrumentType,
        };

        return {
            modalVisible,
            closeModal,
            formData: ref<Record<string, KfConfigValue>>(
                initFormDataByConfig(
                    props.payload.config,
                    props.payload.initValue,
                ),
            ),

            ...props.payload,

            inputTypeToTradeConfig,
        };
    },

    computed: {
        titleResolved() {
            return `${this.type === 'add' ? '添加' : '设置'} ${this.title}`;
        },
    },

    methods: {
        handleConfirm() {
            this.closeModal();
        },

        handleClose() {
            this.closeModal();
        },

        getKfTradeValueName(
            data: Record<number | string | symbol, KfTradeValueCommonData>,
            key: number | string,
        ) {
            return data[key].name;
        },

        handleSelectFile(targetKey: string) {
            dialog
                .showOpenDialog({
                    properties: ['openFile'],
                })
                .then((res) => {
                    const { filePaths } = res;
                    if (filePaths.length) {
                        this.formData[targetKey] = filePaths[0];
                        //     this.$refs.extForm.validateField(targetKey); //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
                    }
                });
        },
    },
});
</script>

<style lang="less">
.kf-set-by-config-modal {
    .source-id__txt {
        margin-right: 8px;
    }

    .kf-form-item__warp {
        &.file {
            display: flex;
            justify-content: space-between;
            align-items: top;

            span.file-path {
                flex: 1;
                word-break: break-word;
                padding-right: 20px;
                box-sizing: border-box;
            }

            button {
                width: 40px;
            }
        }
    }
}
</style>
