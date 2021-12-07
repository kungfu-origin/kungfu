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
    KfCategoryTypes,
    SourceData,
} from '@kungfu-trader/kungfu-js-api/typings';
import { defineComponent, ref, PropType } from 'vue';

import {
    getInstrumentTypeData,
    getSourceDataList,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useModalVisible } from '@renderer/assets/methods/uiUtils';

export default defineComponent({
    props: {
        visible: {
            type: Boolean,
            default: false,
        },

        sourceType: {
            type: String as PropType<KfCategoryTypes>,
            default: 'td',
        },
    },

    setup(props, context) {
        const selectedSource = ref<string>('');
        const sourceDataList = ref<SourceData[]>([]);

        const { modalVisible, closeModal } = useModalVisible(props, context);

        return {
            modalVisible,
            closeModal,

            selectedSource,
            sourceDataList,

            getInstrumentTypeData,
            InstrumentTypeEnum,
        };
    },

    mounted() {
        const extConfigs = this.$useGlobalStore().extConfigs || {};
        this.sourceDataList = getSourceDataList(extConfigs, this.sourceType);

        if (this.selectedSource === '') {
            if (this.sourceDataList.length) {
                this.selectedSource = this.sourceDataList[0].name;
            }
        }
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
