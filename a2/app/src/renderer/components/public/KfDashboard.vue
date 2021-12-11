<template>
    <div class="kf-dashboard__warp kf-translateZ">
        <div class="kf-dashboard__header">
            <slot name="header"></slot>
        </div>
        <div ref="kfDashboardBody" class="kf-dashboard__body">
            <slot></slot>
        </div>
    </div>
</template>
<script lang="ts">
import { filter } from 'rxjs';
import { defineComponent, nextTick } from 'vue';

export default defineComponent({
    name: 'KfDashboard',

    mounted() {
        nextTick().then(() => {
            this.$emit('boardSizeChange', this.getBodyWidthHeight());
        });

        if (this.$bus) {
            this.$bus
                .pipe(filter((e: KfEvent) => e.tag === 'resize'))
                .subscribe(() => {
                    this.$emit('boardSizeChange', this.getBodyWidthHeight());
                });
        }
    },

    methods: {
        getBodyWidthHeight(): { width: number; height: number } {
            const dashboardBody = this.$refs['kfDashboardBody'] as HTMLElement;
            return {
                width: dashboardBody?.clientWidth || 0,
                height: dashboardBody?.clientHeight || 0,
            };
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
        display: flex;
        justify-content: flex-end;
        align-items: center;
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
