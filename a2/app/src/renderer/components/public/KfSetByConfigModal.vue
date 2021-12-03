<template>
    <a-modal
        class="kf-set-by-config-modal"
        v-model:visible="modalVisible"
        :title="titleResolved"
        @cancel="handleClose"
        @ok="handleConfirm"
    >
        <a-form>
            <a-form-item v-for="item in config?.settings" :key="item.key">
                <a-input v-if="item.type === 'str'"></a-input>
                <a-input-password
                    v-if="item.type === 'password'"
                ></a-input-password>
                <a-input-number v-if="item.type === 'int'"></a-input-number>
                <a-input-number
                    v-if="item.type === 'float'"
                    step="0.0001"
                ></a-input-number>
                <a-input-number
                    v-if="item.type === 'percent'"
                    step="0.0001"
                    :formatter="(value: number) => `${value}%`"
                    :parser="(value: string) => value.replace('%', '')"
                ></a-input-number>
                <a-select v-if="item.type === 'side'">
                    <a-select-option
                        v-for="key in Object.keys(Side)"
                        :key="+key"
                        :value="key"
                    >
                        {{ getKfTradeValueName(Side, +key) }}
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
            </a-form-item>
        </a-form>
    </a-modal>
</template>

<script lang="ts">
import { defineComponent, PropType, ref } from 'vue';

import { modalVisibleComposition } from '@renderer/assets/methods/uiUtils';
import {
    CommissionModeEnum,
    DirectionEnum,
    HedgeFlagEnum,
    InstrumentTypeEnum,
    KfConfigValue,
    KfSelectOption,
    KfTradeValueCommonData,
    OffsetEnum,
    PriceTypeEnum,
    SetKfConfigPayload,
    SideEnum,
    TimeConditionEnum,
    VolumeConditionEnum,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    CommissionMode,
    Direction,
    HedgeFlag,
    InstrumentType,
    Offset,
    PriceType,
    Side,
    TimeCondition,
    VolumeCondition,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

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
                'volumCondition', // select - number
                'timeCodition', // select - number
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

            Side,
            Offset,
            Direction,
            PriceType,
            HedgeFlag,
            VolumeCondition,
            TimeCondition,
            CommissionMode,
            InstrumentType,

            SideEnum,
            OffsetEnum,
            DirectionEnum,
            PriceTypeEnum,
            HedgeFlagEnum,
            VolumeConditionEnum,
            TimeConditionEnum,
            CommissionModeEnum,
            InstrumentTypeEnum,
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
            key: number,
        ) {
            return data[key].name;
        },
    },
});
</script>

<style lang="less">
.kf-set-by-config-modal {
    .source-id__txt {
        margin-right: 8px;
    }
}
</style>
