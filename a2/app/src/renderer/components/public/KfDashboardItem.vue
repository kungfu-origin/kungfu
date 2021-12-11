<template>
    <div class="kf-dashboard-item__warp">
        <slot></slot>
    </div>
</template>
<script lang="ts">
import { defineComponent } from 'vue';
export default defineComponent({});
</script>
<style lang="less">
.kf-dashboard-item__warp {
    padding-left: 8px;

    .ant-btn.ant-btn-sm {
        height: 28px;
    }
}
</style>
