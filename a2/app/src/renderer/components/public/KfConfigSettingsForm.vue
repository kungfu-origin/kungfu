<script setup lang="ts">
import { dialog } from '@electron/remote';
import { DashOutlined } from '@ant-design/icons-vue';
import {
    PriceTypeEnum,
    SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
    numberEnumRadioType,
    numberEnumSelectType,
    stringEnumSelectType,
    useAllKfConfigData,
    useInstruments,
} from '@renderer/assets/methods/uiUtils';
import { getCurrentInstance, reactive, ref, toRefs, watch } from 'vue';
import {
    PriceType,
    Side,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { getIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { RuleObject } from 'ant-design-vue/lib/form';

const props = withDefaults(
    defineProps<{
        formState: Record<string, KungfuApi.KfConfigValue>;
        configSettings: KungfuApi.KfConfigItem[];
        changeType: KungfuApi.ModalChangeType;
        primaryKeyAvoidRepeatCompareExtra?: string;
        primaryKeyAvoidRepeatCompareTarget?: string[];
        layout?: 'horizontal' | 'vertical' | 'inline';
        labelAlign?: 'right' | 'left';
        labelCol?: number;
        wrapperCol?: number;
    }>(),
    {
        formState: () => ({}),
        configSettings: () => [],
        changeType: 'add',
        primaryKeyAvoidRepeatCompareTarget: () => [],
        primaryKeyAvoidRepeatCompareExtra: '',
        layout: 'horizontal',
        labelAlign: 'right',
        labelCol: 6,
        wrapperCol: 16,
    },
);

defineEmits<{
    (
        e: 'update:formState',
        formState: Record<string, KungfuApi.KfConfigValue>,
    ): void;
}>();

const app = getCurrentInstance();
const formRef = ref();
const formState = reactive(props.formState);

watch(formState, (newVal) => {
    app && app.emit('update:formState', newVal);
});

const primaryKeys: string[] = (props.configSettings || [])
    .filter((item) => item.primary)
    .map((item) => item.key);

const {
    searchInstrumnetOptions,
    handleSearchInstrument,
    transformSearchInstrumentResultToInstrument,
} = useInstruments();
const { td } = toRefs(useAllKfConfigData());

function getValidatorType(type: string): 'number' | 'string' {
    const intTypes: string[] = [
        'int',
        ...Object.keys(numberEnumSelectType || {}),
        ...Object.keys(numberEnumRadioType || {}),
    ];
    const floatTypes: string[] = ['float', 'percent'];
    if (intTypes.includes(type)) {
        return 'number';
    } else if (floatTypes.includes(type)) {
        return 'number';
    } else {
        return 'string';
    }
}

function isNumberInputType(type: string): boolean {
    const numberInputTypes: string[] = ['int', 'float', 'percent'];
    return numberInputTypes.includes(type);
}

function primaryKeyValidator(): Promise<void> {
    const combineValue: string = [
        props.primaryKeyAvoidRepeatCompareExtra || '',
        ...primaryKeys.map((key) => formState[key]),
    ]
        .filter((item) => item !== '')
        .join('_');

    if (
        props.primaryKeyAvoidRepeatCompareTarget
            .map((item) => item.toLowerCase())
            .includes(combineValue.toLowerCase())
    ) {
        return Promise.reject(new Error(`${combineValue} 已存在`));
    }

    return Promise.resolve();
}

function noZeroValidator(_rule: RuleObject, value: number): Promise<void> {
    console.log(value, '---');
    if (Number.isNaN(+value)) {
        return Promise.reject(new Error(`请输入非零数字`));
    }

    if (+value === 0) {
        return Promise.reject(new Error(`请输入非零数字`));
    }

    return Promise.resolve();
}

function instrumnetValidator(_rule: RuleObject, value: string): Promise<void> {
    if (!value) {
        return Promise.resolve();
    }

    const instrumentResolved =
        transformSearchInstrumentResultToInstrument(value);
    if (!instrumentResolved) {
        return Promise.reject(new Error('标的错误'));
    }

    return Promise.resolve();
}

function getKfTradeValueName(
    data: Record<number | string | symbol, KungfuApi.KfTradeValueCommonData>,
    key: number | string,
): string {
    return data[key].name;
}

function handleSelectFile(targetKey: string) {
    dialog
        .showOpenDialog({
            properties: ['openFile'],
        })
        .then((res) => {
            const { filePaths } = res;
            if (filePaths.length) {
                formState[targetKey] = filePaths[0];
                formRef.value.validateFields([targetKey]); //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
            }
        });
}

function validate(): Promise<void> {
    return formRef.value.validate();
}

defineExpose({
    validate,
});
</script>
<template>
    <a-form
        class="kf-config-form"
        ref="formRef"
        :model="formState"
        :label-col="{ span: labelCol }"
        :wrapper-col="{ span: wrapperCol }"
        :labelAlign="labelAlign"
        :colon="false"
        :scrollToFirstError="true"
        :layout="layout"
    >
        <a-form-item
            v-for="item in configSettings"
            :key="item.key"
            :label="item.name"
            :name="item.key"
            :extra="item.tip"
            :rules="
                changeType === 'update' && item.primary
                    ? []
                    : [
                          ...(item.required
                              ? [
                                    {
                                        required: item.required,
                                        type: getValidatorType(item.type),
                                        message: item.errMsg || '该项为必填项',
                                        trigger: 'blur',
                                    },
                                ]
                              : [
                                    {
                                        required: false,
                                        type: getValidatorType(item.type),
                                        trigger: 'blur',
                                    },
                                ]),
                          ...(item.required && isNumberInputType(item.type)
                              ? [
                                    {
                                        validator: noZeroValidator,
                                        type: getValidatorType(item.type),
                                        trigger: 'blur',
                                    },
                                ]
                              : []),
                          ...(item.primary
                              ? [
                                    {
                                        validator: primaryKeyValidator,
                                        type: getValidatorType(item.type),
                                        trigger: 'change',
                                    },
                                ]
                              : []),

                          ...(item.type === 'instrument'
                              ? [
                                    {
                                        validator: instrumnetValidator,
                                        type: getValidatorType(item.type),
                                        trigger: 'change',
                                    },
                                ]
                              : []),
                      ]
            "
        >
            <a-input
                v-if="item.type === 'str'"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            ></a-input>
            <a-input-password
                v-else-if="item.type === 'password'"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            ></a-input-password>
            <a-input-number
                v-else-if="item.type === 'int'"
                :formatter="(value: number) => +value"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            ></a-input-number>
            <a-input-number
                v-else-if="item.type === 'float'"
                :precision="4"
                step="0.0001"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            ></a-input-number>
            <a-input-number
                v-else-if="item.type === 'percent'"
                :precision="2"
                step="0.01"
                :formatter="(value: number) => `${value}%`"
                :parser="(value: string) => value.replace('%', '')"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            ></a-input-number>
            <a-radio-group
                v-else-if="item.type === 'side'"
                v-model:value="formState[item.key]"
                :name="item.key"
                :disabled="changeType === 'update' && item.primary"
            >
                <a-radio
                    v-for="key in Object.keys(Side).slice(0, 2)"
                    :value="+key"
                >
                    {{ Side[(+key) as SideEnum ].name }}
                </a-radio>
            </a-radio-group>
            <a-select
                v-else-if="item.type === 'priceType'"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                <a-select-option
                    v-for="key in Object.keys(PriceType).slice(0, 7)"
                    :key="key"
                    :value="+key"
                >
                    {{ PriceType[+key as PriceTypeEnum].name }}
                </a-select-option>
            </a-select>
            <a-radio-group
                v-else-if="numberEnumRadioType[item.type]"
                v-model:value="formState[item.key]"
                :name="item.key"
                :disabled="changeType === 'update' && item.primary"
            >
                <a-radio
                    v-for="key in Object.keys(numberEnumRadioType[item.type])"
                    :value="+key"
                >
                    {{
                        getKfTradeValueName(numberEnumRadioType[item.type], key)
                    }}
                </a-radio>
            </a-radio-group>
            <a-select
                v-else-if="numberEnumSelectType[item.type]"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                {{ item.type }}
                <a-select-option
                    v-for="key in Object.keys(numberEnumSelectType[item.type])"
                    :key="key"
                    :value="+key"
                >
                    {{
                        getKfTradeValueName(
                            numberEnumSelectType[item.type],
                            key,
                        )
                    }}
                </a-select-option>
            </a-select>
            <a-select
                v-else-if="stringEnumSelectType[item.type]"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                {{ item.type }}
                <a-select-option
                    v-for="key in Object.keys(stringEnumSelectType[item.type])"
                    :key="key"
                    :value="key"
                >
                    {{
                        getKfTradeValueName(
                            stringEnumSelectType[item.type],
                            key,
                        )
                    }}
                </a-select-option>
            </a-select>
            <a-select
                v-else-if="item.type === 'select'"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                <a-select-option
                    v-for="option in item.options"
                    :key="option.value"
                    :value="option.value"
                >
                    {{ option.label }}
                </a-select-option>
            </a-select>
            <a-select
                v-else-if="item.type === 'instrument'"
                :disabled="changeType === 'update' && item.primary"
                show-search
                v-model:value="formState[item.key]"
                :filter-option="false"
                :options="searchInstrumnetOptions"
                @search="handleSearchInstrument"
            ></a-select>
            <a-select
                v-else-if="item.type === 'td'"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                <a-select-option
                    v-for="config in td"
                    :key="getIdByKfLocation(config)"
                    :value="getIdByKfLocation(config)"
                >
                    {{ getIdByKfLocation(config) }}
                </a-select-option>
            </a-select>
            <a-switch
                size="small"
                v-else-if="item.type === 'bool'"
                v-model:checked="formState[item.key]"
            ></a-switch>
            <div
                v-else-if="item.type === 'file'"
                class="kf-form-item__warp file"
                :disabled="changeType === 'update' && item.primary"
            >
                <span
                    v-if="formState[item.key]"
                    class="file-path"
                    :title="(formState[item.key] || '').toString()"
                >
                    {{ formState[item.key] }}
                </span>
                <a-button size="small" @click="handleSelectFile(item.key)">
                    <template #icon><DashOutlined /></template>
                </a-button>
            </div>
        </a-form-item>
    </a-form>
</template>
<style lang="less">
.kf-config-form {
    .kf-form-item__warp {
        &.file {
            display: flex;
            justify-content: space-between;
            align-items: top;
            padding-bottom: 4px;

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

    .ant-form-item-control-input-content {
        text-align: left;
    }

    .ant-radio-group {
        .ant-radio-wrapper {
            line-height: 2;
        }
    }
}
</style>
