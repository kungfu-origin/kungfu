<template>
    <div class="kf-state-status">
        <div
            :class="[
                'kf-dot',
                stateStatusData?.color || '',
                isWaveStatus(stateStatusData?.color) ? 'kf-dot-wave' : '',
            ]"
            v-if="stateStatusData && (stateStatusData.level || 0) >= 1"
        ></div>
        {{
            +(stateStatusData?.level || 0) === 0 ? '--' : stateStatusData?.name
        }}
    </div>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';
import {
    AntInKungfuColorTypes,
    KfTradeValueCommonData,
    ProcessStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings/index';
import { getStateStatusData } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

export default defineComponent({
    name: 'KfProcessStatus',
    props: {
        statusName: {
            type: String as PropType<ProcessStatusTypes | undefined>,
        },
    },

    computed: {
        stateStatusData(): KfTradeValueCommonData | undefined {
            return getStateStatusData(this.statusName);
        },
    },

    methods: {
        isWaveStatus(statusColor: AntInKungfuColorTypes | undefined) {
            if (statusColor === undefined) return false;
            if (statusColor === 'kf-color-error') return false;
            return true;
        },
    },
});
</script>

<style lang="less"></style>
