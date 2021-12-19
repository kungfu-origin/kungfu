<script setup lang="ts">
import { dialog } from '@electron/remote';
import { DashOutlined } from '@ant-design/icons-vue';
import {
    KfConfigItem,
    KfConfigValue,
    KfTradeValueCommonData,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    numberEnumInputType,
    stringEnumInputType,
} from '@renderer/assets/methods/uiUtils';
import { getCurrentInstance, reactive, ref, watch } from 'vue';

const props = withDefaults(
    defineProps<{
        formState: Record<string, KfConfigValue>;
        configSettings: KfConfigItem[];
        changeType: ModalChangeType;
        primaryKeyAvoidRepeatCompareExtra?: string;
        primaryKeyAvoidRepeatCompareTarget?: string[];
    }>(),
    {
        formState: () => ({}),
        configSettings: () => [],
        changeType: 'add',
        primaryKeyAvoidRepeatCompareTarget: () => [],
        primaryKeyAvoidRepeatCompareExtra: '',
    },
);

defineEmits<{
    (e: 'update:formState', formState: Record<string, KfConfigValue>): void;
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

function getValidatorType(type: string): 'integer' | 'float' | 'string' {
    const intTypes: string[] = [
        'int',
        ...Object.keys(numberEnumInputType || {}),
    ];
    const floatTypes: string[] = ['float', 'percent'];
    if (intTypes.includes(type)) {
        return 'integer';
    } else if (floatTypes.includes(type)) {
        return 'float';
    } else {
        return 'string';
    }
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

function getKfTradeValueName(
    data: Record<number | string | symbol, KfTradeValueCommonData>,
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
        :label-col="{ span: 6 }"
        :wrapper-col="{ span: 16 }"
        :colon="false"
        :scrollToFirstError="true"
    >
        <a-form-item
            v-for="item in configSettings"
            :key="item.key"
            :label="item.name"
            :name="item.key"
            :required="item.required"
            :rules="
                changeType === 'update' && item.primary
                    ? []
                    : [
                          {
                              required: item.required,
                              type: getValidatorType(item.type),
                              message: '该项为必填项',
                              trigger: 'blur',
                          },
                          ...(item.primary
                              ? [
                                    {
                                        validator: primaryKeyValidator,
                                        trigger: 'change',
                                    },
                                ]
                              : []),
                      ]
            "
        >
            <a-input
                v-if="item.type === 'str'"
                :formatter="(value: number) => +value"
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
            <a-select
                v-else-if="numberEnumInputType[item.type]"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                {{ item.type }}
                <a-select-option
                    v-for="key in Object.keys(numberEnumInputType[item.type])"
                    :key="key"
                    :value="+key"
                >
                    {{
                        getKfTradeValueName(numberEnumInputType[item.type], key)
                    }}
                </a-select-option>
            </a-select>
            <a-select
                v-else-if="stringEnumInputType[item.type]"
                v-model:value="formState[item.key]"
                :disabled="changeType === 'update' && item.primary"
            >
                {{ item.type }}
                <a-select-option
                    v-for="key in Object.keys(stringEnumInputType[item.type])"
                    :key="key"
                    :value="key"
                >
                    {{
                        getKfTradeValueName(stringEnumInputType[item.type], key)
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

            <div
                v-else-if="item.type === 'file'"
                class="kf-form-item__warp file"
                :disabled="changeType === 'update' && item.primary"
            >
                <span
                    class="file-path"
                    v-if="formState[item.key]"
                    :title="(formState[item.key] || '').toString()"
                >
                    {{ formState[item.key] }}
                </span>
                <a-button size="mini" @click="handleSelectFile(item.key)">
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
