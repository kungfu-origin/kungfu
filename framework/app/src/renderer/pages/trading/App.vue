<script lang="ts" setup>
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import TradingTaskItem from './components/TradingTaskItem.vue';
import { nextTick, onMounted, ref } from 'vue';
import { removeLoadingMask } from '../../assets/methods/uiUtils';
import { ipcEmitDataByName } from '../../ipcMsg/emitter';

const strategyStatesList = ref<KungfuApi.StrategyStateListItem[]>([]);

onMounted(() => {
  ipcEmitDataByName('strategyStates').then(({ data }) => {
    nextTick().then(() => {
      strategyStatesList.value = dealStrategyStates(data);

      removeLoadingMask();
    });
  });
});

const dealStrategyStates = (
  data: Record<string, KungfuApi.StrategyStateData>,
): KungfuApi.StrategyStateListItem[] => {
  return Object.keys(data).reduce((strategyStatesItem, key) => {
    if (data[key]?.info_a) {
      strategyStatesItem.push({
        ...JSON.parse(data[key]?.info_a),
        value: data[key]?.value,
        process_id: key,
      });
    }
    return strategyStatesItem;
  }, [] as KungfuApi.StrategyStateListItem[]);
};
</script>

<template>
  <a-layout>
    <div class="kf-trading-view_warp">
      <KfDashboard>
        <div class="task-content">
          <div class="trade-item" v-for="item in strategyStatesList">
            <TradingTaskItem :strategyStatesItem="item"></TradingTaskItem>
          </div>
        </div>
      </KfDashboard>
    </div>
  </a-layout>
</template>

<style lang="less">
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/base.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/public.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/coverAnt.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/variables.less';

#app {
  width: 100%;
  height: 100%;
  font-family: Consolas, Monaco, Lucida Console, Liberation Mono,
    DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;

  .ant-layout {
    height: 100%;
    background: #141414;

    .kf-dashboard__header {
      .kf-dashboard-item__warp {
        .search-in-table__warp {
          display: flex;
          justify-content: flex-start;
          align-items: center;
          font-size: 12px;
          margin-right: 4px;

          .search-int-table__item {
            margin: 0 4px;
          }
        }
      }
    }

    .kf-dashboard__body {
      background: #141414;
      padding: 8px;
      box-sizing: border-box;
      border-radius: 4px;
    }
    .ant-card-body {
      padding: 24px;
    }
  }
  .task-content {
    width: 100%;
    padding: 0 8px 8px 8px;
    display: flex;
    box-sizing: border-box;
    flex-wrap: wrap;

    .trade-item {
      width: 33.33%;
      box-sizing: border-box;
      padding: 6px;
    }
  }

  .kf-trading-view_warp {
    height: 100%;
    width: 100%;
    padding: 0 8px 8px 8px;
  }

  .kf-log-line {
    text-align: left;
    font-size: 14px;
    user-select: text;
    padding-bottom: 4px;
    line-height: 1.5;

    .error {
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .debug {
      color: @blue-6;
      font-weight: bold;
    }

    .info {
      color: @green2-base;
      font-weight: bold;
    }

    .warning {
      color: @orange-6;
      font-weight: bold;
    }

    .trace {
      color: @cyan-6;
      font-weight: bold;
    }

    .critical {
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .search-keyword {
      background: fade(@white, 70%);
      color: #000;
      font-weight: normal;

      &.current-search-pointer {
        background: @primary-color;
        color: #fff;
      }
    }
  }
}
</style>
