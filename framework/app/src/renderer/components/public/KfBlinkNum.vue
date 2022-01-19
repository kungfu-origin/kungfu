<script lang="ts" setup>
import { computed, ref, watch } from 'vue';

const props = withDefaults(
  defineProps<{
    num: string | number;
    mode?: 'normal' | 'compare-zero';
    blinkType?: 'background' | 'color';
  }>(),
  {
    num: '--',
    mode: 'normal',
    blinkType: 'background',
  },
);

const classname = ref<string>('');
const compareZeroClassname = computed(() => {
  if (props.mode !== 'compare-zero') {
    return '';
  }

  const num = +parseFloat(props.num + '');

  if (Number.isNaN(num)) {
    return '';
  }

  if (+num > 0) {
    return 'color-red';
  } else if (+num < 0) {
    return 'color-green';
  } else {
    return '';
  }
});

watch(
  () => props.num,
  (newVal, oldVal) => {
    const newNum = +parseFloat(newVal + '');
    const oldNum = +parseFloat(oldVal + '');

    if (Number.isNaN(newNum) || Number.isNaN(oldNum)) {
      classname.value = '';
      return;
    }

    if (props.mode === 'compare-zero') {
      if (+newNum > 0) {
        classname.value = 'blink-red';
      } else if (+newNum === 0) {
        classname.value = '';
      } else {
        classname.value = 'blink-green';
      }
    }

    if (props.mode === 'normal') {
      if (newNum > oldNum) {
        classname.value = 'blink-red';
      } else if (newNum === oldNum) {
        classname.value = '';
      } else {
        classname.value = 'blink-green';
      }
    }
  },
);
</script>
<template>
  <div
    :class="['kf-blink-num', mode, classname, blinkType, compareZeroClassname]"
    :key="num"
  >
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

  &.color {
    &.blink-red {
      color: @red-6;
      animation: nanoRedColorBlink 0.7s 1;
    }

    &.blink-green {
      color: @green-6;
      animation: nanoGreenColorBlink 0.7s 1;
    }
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
