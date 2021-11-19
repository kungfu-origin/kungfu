<template>
    <div class="kf-index__warp">
        <KfLayout>
            <KfDragRow>
                <KfDragCol>
                    <KfDragRow>
                        <KfDragCol></KfDragCol>
                    </KfDragRow>
                    <KfDragRow>
                        <KfDragCol></KfDragCol>
                        <KfDragCol></KfDragCol>
                    </KfDragRow>
                </KfDragCol>
                <KfDragCol></KfDragCol>
                <KfDragCol></KfDragCol>
            </KfDragRow>
        </KfLayout>
    </div>
</template>

<script lang="ts">
import { defineComponent } from '@vue/runtime-core';
import KfLayout from '@/components/global/KfLayout.vue';
import KfDragRow from '@/components/global/KfDragRow.vue';
import KfDragCol from '@/components/global/KfDragCol.vue';

export default defineComponent({
    name: 'Index',

    components: {
        KfLayout,
        KfDragRow,
        KfDragCol,
    },
});
</script>

<style lang="less">
.kf-index__warp {
    height: 100%;
    width: 100%;
}
</style>
