<script lang="ts" setup>
import { toRefs } from 'vue';
import { strategyStatesTrans } from '../translate';

const props = defineProps<{
  strategyStatesItem: KungfuApi.StrategyStateListItem;
}>();

const { strategyStatesItem } = toRefs(props);
</script>

<template>
  <div class="trade-task">
    <a-card hoverable :title="strategyStatesItem.process_id">
      <p class="item" v-for="item in Object.keys(strategyStatesItem)">
        <span v-if="strategyStatesTrans[item]">
          {{ strategyStatesTrans[item] }}: {{ strategyStatesItem[item] }}
        </span>
      </p>
    </a-card>
  </div>
</template>

<style lang="less">
.trade-task {
  padding: 4px;
  text-align: left;
  .title {
    font-size: 18px;
  }
  .item {
    font-size: 14px;
  }
}
</style>
