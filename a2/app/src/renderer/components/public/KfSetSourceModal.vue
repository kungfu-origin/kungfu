<template>
    <a-modal
        class="kf-set-source-modal"
        :width="480"
        v-model:visible="modalVisible"
        title="选择柜台API"
        @cancel="handleClose"
        @ok="handleConfirm"
    >
        <a-radio-group v-model:value="selectedSource">
            <a-radio
                :value="item.name"
                :key="item.name"
                v-for="item in sourceDataList"
                value="1"
                :style="radioStyle"
            >
                <span class="source-id__txt">{{ item.name }}</span>
                <a-tag
                    v-for="(sourceInstrumentType, index) in item.type"
                    :key="index"
                    :color="
                        getInstrumentTypeData(
                            InstrumentTypeEnum[sourceInstrumentType],
                        ).color
                    "
                >
                    {{
                        getInstrumentTypeData(
                            InstrumentTypeEnum[sourceInstrumentType],
                        ).name
                    }}
                </a-tag>
            </a-radio>
        </a-radio-group>
    </a-modal>
</template>

<script lang="ts">
import {
    InstrumentTypeEnum,
    KfCategoryEnum,
    SourceData,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    defineComponent,
    ref,
    PropType,
    onMounted,
    getCurrentInstance,
} from 'vue';

import {
    getInstrumentTypeData,
    getSourceDataList,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { modalVisibleComposition } from '@renderer/assets/methods/uiUtils';

export default defineComponent({
    props: {
        visible: {
            type: Boolean,
            default: false,
        },

        sourceType: {
            type: String as PropType<KfCategoryEnum.td | KfCategoryEnum.md>,
            default: KfCategoryEnum.td,
        },
    },

    setup(props, context) {
        const selectedSource = ref<string>('');
        const sourceDataList = ref<SourceData[]>([]);

        const { modalVisible, closeModal } = modalVisibleComposition(
            props,
            context,
        );

        const radioStyle = {
            height: '36px',
            'line-height': '36px',
            'font-size': '16px',
            'min-width': '45%',
        };

        onMounted(() => {
            const app = getCurrentInstance();
            if (app?.proxy) {
                const extConfigs = app.proxy.$useGlobalStore().extConfigs || {};
                sourceDataList.value = getSourceDataList(
                    extConfigs,
                    props.sourceType,
                );

                if (selectedSource.value === '') {
                    if (sourceDataList.value.length) {
                        selectedSource.value = sourceDataList.value[0].name;
                    }
                }
            }
        });

        return {
            modalVisible,
            closeModal,

            selectedSource,
            sourceDataList,
            radioStyle,
            getInstrumentTypeData,
            InstrumentTypeEnum,
        };
    },

    methods: {
        handleConfirm() {
            this.$emit('confirm', this.selectedSource);
            this.closeModal();
        },

        handleClose() {
            this.closeModal();
        },
    },
});
</script>

<style lang="less">
.kf-set-source-modal {
    .source-id__txt {
        margin-right: 8px;
    }
}
</style>
