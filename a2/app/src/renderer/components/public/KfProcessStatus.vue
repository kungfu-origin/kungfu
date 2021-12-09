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
            type: String as PropType<ProcessStatusTypes>,
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

<style lang="less">
.keyframes(@name, @rules) {
    @-webkit-keyframes @name {
        @rules();
    }

    @-moz-keyframes @name {
        @rules();
    }

    @-ms-keyframes @name {
        @rules();
    }

    @-o-keyframes @name {
        @rules();
    }

    @keyframes @name {
        @rules();
    }
}

.keyframes(KfStatusRunning, {
    0% {
        width: 6px;
        height: 6px;
        box-shadow: 0 0 0;
    }
    100% {
        width: 10px;
        height: 10px;
        box-shadow: 0 0 6px @primary-color
    }
});

.keyframes(KfStatusWaiting, {
    0% {
        width: 6px;
        height: 6px;
        box-shadow: 0 0 0;
    }
    100% {
        width: 10px;
        height: 10px;
        box-shadow: 0 0 6px fade(@white, 50%);
    }
});

.kf-state-status {
    .kf-dot {
        width: 6px;
        height: 6px;
        display: inline-block;
        border-radius: 50%;
        vertical-align: middle;
        margin-right: 4px;

        &.kf-color-running {
            background: @primary-color;

            &.kf-dot-wave {
                &::after {
                    border: 1px solid @primary-color;
                    animation: KfStatusRunning 1s infinite ease-in-out;
                }
            }
        }

        &.kf-color-waiting {
            background: fade(@white, 50%);

            &.kf-dot-wave {
                &::after {
                    border: 1px solid fade(@white, 50%);
                    animation: KfStatusWaiting 1s infinite ease-in-out;
                }
            }
        }

        &.kf-color-error {
            background: @red-7;
        }

        &.kf-dot-wave {
            position: relative;

            &::after {
                position: absolute;
                top: 50%;
                left: 50%;
                transform: translateY(-50%) translateX(-50%);
                width: 6px;
                height: 6px;
                border-radius: 50%;
                content: '';
            }
        }
    }
}
</style>
