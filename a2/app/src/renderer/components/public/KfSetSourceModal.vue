<script setup lang="ts">
import {
    InstrumentTypeEnum,
    KfCategoryTypes,
    SourceData,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    defineComponent,
    ref,
    PropType,
    onMounted,
    computed,
    getCurrentInstance,
} from 'vue';

import {
    getInstrumentTypeData,
    getSourceDataList,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useExtConfigsRelated,
    useModalVisible,
} from '@renderer/assets/methods/uiUtils';

const props = withDefaults(
    defineProps<{
        visible: boolean;
        sourceType: KfCategoryTypes;
    }>(),
    {
        visible: false,
        sourceType: 'td',
    },
);

defineEmits<{
    (e: 'confirm', sourceId: string): void;
    (e: 'update:visible', visible: boolean): void;
    (e: 'close'): void;
}>();

const app = getCurrentInstance();
const { extConfigs } = useExtConfigsRelated();
const selectedSource = ref<string>('');
const sourceDataList = computed(() => {
    return getSourceDataList(extConfigs.value, props.sourceType);
});

const { modalVisible, closeModal } = useModalVisible(props.visible);

onMounted(() => {
    if (selectedSource.value === '') {
        if (sourceDataList.value.length) {
            selectedSource.value = sourceDataList.value[0].name;
        }
    }
});

function handleConfirm() {
    app && app.emit('confirm', selectedSource.value);
    closeModal();
}
</script>
<template>
    <a-modal
        class="kf-set-source-modal"
        :width="480"
        v-model:visible="modalVisible"
        title="选择柜台API"
        :destroyOnClose="true"
        @cancel="closeModal"
        @ok="handleConfirm"
    >
        <a-radio-group v-model:value="selectedSource">
            <a-radio
                :value="item.name"
                :key="item.name"
                v-for="item in sourceDataList"
                value="1"
                :style="{
                    height: '36px',
                    'line-height': '36px',
                    'font-size': '16px',
                    'min-width': '45%',
                }"
            >
                <span class="source-id__txt">{{ item.name }}</span>
                <a-tag
                    v-for="(sourceInstrumentType, index) in item.type"
                    :key="index"
                    :color="getInstrumentTypeData(sourceInstrumentType).color"
                >
                    {{ getInstrumentTypeData(sourceInstrumentType).name }}
                </a-tag>
            </a-radio>
        </a-radio-group>
    </a-modal>
</template>
<style lang="less">
.kf-set-source-modal {
    .source-id__txt {
        margin-right: 8px;
    }
}
</style>
