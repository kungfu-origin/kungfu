<script lang="ts" setup>
import { ref, watch } from 'vue';

const props = withDefaults(
    defineProps<{
        num: string | number;
        mode?: 'normal' | 'compare-zero';
    }>(),
    {
        num: '--',
        mode: 'normal',
    },
);

const classname = ref<string>('');

watch(
    () => props.num,
    (newVal, oldVal) => {
        const newNum = +Number(newVal);
        const oldNum = +Number(oldVal);

        if (Number.isNaN(newNum) || Number.isNaN(oldNum)) {
            classname.value = '';
        }

        if (props.mode === 'compare-zero') {
            if (+newNum > 0) {
                classname.value = 'blink-red';
            } else if (+newNum === 0) {
                classname.value = '';
            } else {
                classname.value = 'blink-green';
            }
            return;
        }

        if (props.mode === 'normal') {
            if (newNum > oldNum) {
                classname.value = 'blink-red';
            } else if (newNum === oldNum) {
                classname.value = '';
            } else {
                classname.value = 'blink-green';
            }
            return;
        }
    },
);
</script>
<template>
    <div :class="['kf-blink-num', mode, classname]" :key="num">
        <span>{{ num }}</span>
    </div>
</template>
<style lang="less">
.kf-blink-num {
    display: inline-block;
    width: 100%;
    height: 100%;
    position: absolute;
    top: 0;
    left: 0;
    display: table-cell;
    box-sizing: border-box;
    overflow-wrap: break-word;
    transform: translateZ(0);

    &.blink-red {
        animation: nanoRedBlink 1.1s 1;
    }

    &.blink-green {
        animation: nanoGreenBlink 1.1s 1;
    }

    span {
        vertical-align: middle;
    }
}

.ant-table-cell {
    .kf-blink-num {
        padding: 8px;
    }
}

.kf-table-cell {
    .kf-blink-num {
        padding: 0 6px;
    }
}
</style>
