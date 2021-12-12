<template>
    <a-modal
        :width="width"
        class="kf-set-by-config-modal"
        v-model:visible="modalVisible"
        :title="titleResolved"
        @cancel="handleClose"
        @ok="handleConfirm"
    >
        <a-form
            ref="formRef"
            :model="formState"
            :label-col="{ span: 6 }"
            :wrapper-col="{ span: 16 }"
            :colon="false"
            :scrollToFirstError="true"
        >
            <a-form-item
                v-for="item in config?.settings"
                :key="item.key"
                :label="item.name"
                :name="item.key"
                :rules="
                    payload.type === 'update' && item.primary
                        ? []
                        : [
                              {
                                  required: item.required,
                                  message: '该项为必填项',
                                  trigger: 'change',
                              },
                              ...(item.validator || [
                                  {
                                      validator: defaultValidator,
                                  },
                              ]),
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
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                ></a-input>
                <a-input-password
                    v-if="item.type === 'password'"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                ></a-input-password>
                <a-input-number
                    v-if="item.type === 'int'"
                    :precision="0"
                    step="1"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                ></a-input-number>
                <a-input-number
                    v-if="item.type === 'float'"
                    :precision="4"
                    step="0.0001"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                ></a-input-number>
                <a-input-number
                    v-if="item.type === 'percent'"
                    :precision="2"
                    step="0.01"
                    :formatter="(value: number) => `${value}%`"
                    :parser="(value: string) => value.replace('%', '')"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                ></a-input-number>
                <a-select
                    v-if="numberEnumInputType[item.type]"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                >
                    {{ item.type }}
                    <a-select-option
                        v-for="key in Object.keys(
                            numberEnumInputType[item.type],
                        )"
                        :key="key"
                        :value="+key"
                    >
                        {{
                            getKfTradeValueName(
                                numberEnumInputType[item.type],
                                key,
                            )
                        }}
                    </a-select-option>
                </a-select>
                <a-select
                    v-if="stringEnumInputType[item.type]"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
                >
                    {{ item.type }}
                    <a-select-option
                        v-for="key in Object.keys(
                            stringEnumInputType[item.type],
                        )"
                        :key="key"
                        :value="key"
                    >
                        {{
                            getKfTradeValueName(
                                stringEnumInputType[item.type],
                                key,
                            )
                        }}
                    </a-select-option>
                </a-select>
                <a-select
                    v-if="item.type === 'select'"
                    v-model:value="formState[item.key]"
                    :disabled="payload.type === 'update' && item.primary"
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
                    v-if="item.type === 'file'"
                    class="kf-form-item__warp file"
                    :disabled="payload.type === 'update' && item.primary"
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
    </a-modal>
</template>

<script lang="ts">
import { defineComponent, PropType, reactive, ref } from 'vue';
import { DashOutlined } from '@ant-design/icons-vue';

import {
    initFormDataByConfig,
    numberEnumInputType,
    stringEnumInputType,
    useModalVisible,
} from '@renderer/assets/methods/uiUtils';
import {
    KfConfigValue,
    KfTradeValueCommonData,
    SetKfConfigPayload,
    StrategyData,
} from '@kungfu-trader/kungfu-js-api/typings';

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

        width: {
            type: Number,
            default: 520,
        },

        primaryKeyCompareTarget: {
            type: Array as PropType<string[]>,
            default: [],
        },

        primaryKeyCompareExtra: {
            type: String as PropType<string>,
            default: '',
        },
    },

    components: {
        DashOutlined,
    },

    setup(props, context) {
        const { modalVisible, closeModal } = useModalVisible(props, context);

        const primaryKeys: string[] = (props.payload.config?.settings || [])
            .filter((item) => item.primary)
            .map((item) => item.key);

        const formState = reactive<Record<string, KfConfigValue>>(
            initFormDataByConfig(props.payload.config, props.payload.initValue),
        );

        return {
            modalVisible,
            closeModal,

            formRef: ref(),
            formState,

            ...props.payload,

            defaultValidator: () => Promise.resolve(),
            primaryKeyValidator: (): Promise<void> => {
                const combineValue: string = [
                    props.primaryKeyCompareExtra || '',
                    ...primaryKeys.map((key) => formState[key]),
                ].join('_');
                if (
                    props.primaryKeyCompareTarget
                        .map((item) => item.toLowerCase())
                        .includes(combineValue.toLowerCase())
                ) {
                    return Promise.reject(new Error(`${combineValue} 已存在`));
                }

                return Promise.resolve();
            },

            numberEnumInputType: numberEnumInputType,
            stringEnumInputType: stringEnumInputType,
        };
    },

    computed: {
        titleResolved() {
            return `${this.type === 'add' ? '添加' : '设置'} ${this.title}`;
        },
    },

    methods: {
        handleConfirm() {
            this.formRef
                .validate()
                .then(() => {
                    this.$emit(
                        'confirm',
                        this.formState as unknown as StrategyData,
                    );
                })
                .then(() => {
                    this.closeModal();
                })
                .catch((err: Error) => {
                    console.error(err);
                });
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
                        this.formState[targetKey] = filePaths[0];
                        this.formRef.validateFields([targetKey]); //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
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
