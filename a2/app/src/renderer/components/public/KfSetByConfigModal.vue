<script setup lang="ts">
import { computed, getCurrentInstance, reactive, ref } from 'vue';
import {
    initFormDataByConfig,
    useModalVisible,
} from '@renderer/assets/methods/uiUtils';
import {
    KfConfigValue,
    SetKfConfigPayload,
} from '@kungfu-trader/kungfu-js-api/typings';

import { buildIdByKeysFromKfConfigSettings } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import KfConfigForm from '@renderer/components/public/KfConfigForm.vue';

const props = withDefaults(
    defineProps<{
        visible: boolean;
        payload: SetKfConfigPayload;
        width?: number;
        primaryKeyAvoidRepeatCompareExtra?: string;
        primaryKeyAvoidRepeatCompareTarget?: string[];
    }>(),
    {
        visible: false,
        payload: () => ({} as SetKfConfigPayload),
        width: 520,
        primaryKeyAvoidRepeatCompareTarget: () => [],
        primaryKeyAvoidRepeatCompareExtra: '',
    },
);

defineEmits<{
    (
        e: 'confirm',
        formState: Record<string, KfConfigValue>,
        idByPrimaryKeys: string,
        changeType: ModalChangeType,
    ): void;
    (e: 'update:visible', visible: boolean): void;
    (e: 'close'): void;
}>();

const app = getCurrentInstance();
const { modalVisible, closeModal } = useModalVisible(props);
const formRef = ref();
const formState = reactive<Record<string, KfConfigValue>>(
    initFormDataByConfig(
        props.payload.config?.settings || [],
        props.payload.initValue,
    ),
);

const titleResolved = computed(() => {
    return `${props.payload.type === 'add' ? '添加' : '设置'} ${
        props.payload.title
    }`;
});

function handleConfirm() {
    formRef.value
        .validate()
        .then(() => {
            const primaryKeys: string[] = (props.payload.config?.settings || [])
                .filter((item) => item.primary)
                .map((item) => item.key);

            const idByPrimaryKeys = buildIdByKeysFromKfConfigSettings(
                formState,
                primaryKeys,
            );

            app &&
                app.emit(
                    'confirm',
                    formState,
                    idByPrimaryKeys,
                    props.payload.type,
                );
        })
        .then(() => {
            closeModal();
        })
        .catch((err: Error) => {
            console.error(err);
        });
}

function handleClose() {
    closeModal();
}
</script>
<template>
    <a-modal
        :width="width"
        class="kf-set-by-config-modal"
        v-model:visible="modalVisible"
        :title="titleResolved"
        :destroyOnClose="true"
        @cancel="handleClose"
        @ok="handleConfirm"
    >
        <KfConfigForm
            ref="formRef"
            v-model:formState="formState"
            :configSettings="payload.config?.settings || []"
            :changeType="payload.type"
            :primaryKeyAvoidRepeatCompareTarget="
                primaryKeyAvoidRepeatCompareTarget
            "
            :primaryKeyAvoidRepeatCompareExtra="
                primaryKeyAvoidRepeatCompareExtra
            "
        ></KfConfigForm>
    </a-modal>
</template>

<style lang="less"></style>
