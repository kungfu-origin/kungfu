<template>
    <a-modal
        class="kf-set-by-config-modal"
        v-model:visible="modalVisible"
        title="选择柜台API"
        @cancel="handleClose"
        @ok="handleConfirm"
    ></a-modal>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

import { useModalVisible } from '@renderer/assets/methods/kfUiUtils';

export default defineComponent({
    props: {
        visible: {
            type: Boolean,
            default: false,
        },

        modalData: {
            type: Object,
            default: () => ({}),
        },
    },

    setup(props, context) {
        const { modalVisible, closeModal } = useModalVisible(props, context);

        return {
            modalVisible,
            closeModal,
        };
    },

    methods: {
        handleConfirm() {
            this.closeModal();
        },

        handleClose() {
            this.closeModal();
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
