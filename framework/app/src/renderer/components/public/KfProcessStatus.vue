<template>
  <div class="kf-state-status">
    <div
      :class="[
        'kf-dot',
        stateStatusData?.color || '',
        isWaveStatus(stateStatusData?.color) ? 'kf-dot-wave' : '',
      ]"
      v-if="stateStatusData && (stateStatusData.level || 0) !== 0"
    ></div>
    {{ +(stateStatusData?.level || 0) === 0 ? '--' : stateStatusData?.name }}
  </div>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';
import { getStateStatusData } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { ProcessStatusTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';

export default defineComponent({
  name: 'KfProcessStatus',
  props: {
    statusName: {
      type: String as PropType<ProcessStatusTypes | undefined>,
    },
  },

  computed: {
    stateStatusData(): KungfuApi.KfTradeValueCommonData | undefined {
      return getStateStatusData(this.statusName);
    },
  },

  methods: {
    isWaveStatus(statusColor: KungfuApi.AntInKungfuColorTypes | undefined) {
      if (statusColor === undefined) return false;
      if (statusColor === 'kf-color-error') return false;
      return true;
    },
  },
});
</script>

<style lang="less"></style>
