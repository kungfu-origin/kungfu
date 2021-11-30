<template>
    <div class="kf-dashboard__warp kf-translateZ">
        <div class="kf-dashboard__header">
            <slot name="header"></slot>
        </div>
        <div ref="body" class="kf-dashboard__body">
            <slot></slot>
        </div>
    </div>
</template>
<script lang="ts">
import { defineComponent, ref } from 'vue';

export default defineComponent({
    name: 'KfDashboard',

    setup() {
        return {
            bodyHeight: ref(0),
        };
    },

    mounted() {
        this.bodyHeight = this.getBodyHeight();
        this.$emit('bodyHeight', this.bodyHeight);
    },

    methods: {
        getBodyHeight(): number {
            return (this.$refs['body'] as HTMLElement)?.clientHeight || 0;
        },
    },
});
</script>
<style lang="less">
.kf-dashboard__warp {
    height: 100%;
    width: 100%;
    display: flex;
    flex-direction: column;
    padding: 4px 8px;
    box-sizing: border-box;
    overflow: hidden;

    .kf-dashboard__header {
        height: 32px;
        line-height: 32px;
        margin-bottom: 4px;
    }

    .kf-dashboard__body {
        flex: 1;
        overflow: hidden;

        .ant-table-wrapper {
            height: 100%;
        }
    }
}
</style>
