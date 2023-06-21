<script setup lang="ts">
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';

import {
  useDashboardBodySize,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { getColumns } from './config';
import { getCurrentInstance, onBeforeUnmount, onMounted, ref } from 'vue';

import { useCurrentGlobalKfLocation } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { ArrowRightOutlined } from '@ant-design/icons-vue';

const { t } = VueI18n.global;

const app = getCurrentInstance();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(window.watcher);
const columns = getColumns();

let tableList = ref<KungfuApi.TimeKeyValue[]>([]);
let tableListResolved = ref<KungfuApi.TransferRecordResolved[]>([]);
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.TransferRecordResolved>(tableListResolved, [
    'update_time',
    'source',
    'target',
    'amount',
    'trans_type',
  ]);

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (!currentGlobalKfLocation.value) return;
        const source = watcher.getLocationUID(currentGlobalKfLocation.value);
        tableList.value = (
          watcher.ledger[
            'TimeKeyValue'
          ] as KungfuApi.DataTable<KungfuApi.TimeKeyValue>
        )
          .filter('tag_a', 'FundTrans')
          .filter('source', source)
          .list();
        tableListResolved.value = tableList.value.map(
          (item: KungfuApi.TimeKeyValue) => {
            const value = JSON.parse(item.value);
            const result: KungfuApi.TransferRecordResolved = {
              update_time: BigInt(value.update_time) || 0n,
              source: value.source || '--',
              target: value.target || '--',
              amount: value.amount || 0,
              trans_type: value.key,
            };
            return result;
          },
        );
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});
</script>
<template>
  <div class="kf-market-data__warp">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template #header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableData"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :empty-text="$t('empty_text')"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.TransferRecordResolved,
          }"
        >
          <template v-if="column.dataIndex === 'update_time'">
            <div>{{ dealKfTime(record.update_time, true) }}</div>
          </template>
          <template v-if="column.dataIndex === 'trans_type'">
            <div v-if="record.trans_type === 'FundTransBetweenNodes'">
              <span class="trans-name__txt">HTS</span>
              <ArrowRightOutlined style="margin-right: 8px; font-size: 10px" />
              <span class="trans-name__txt">HTS</span>
            </div>
            <div v-if="record.trans_type === 'FundTransIn'">
              <span class="trans-name__txt">
                {{ t('fund_trans.centralized_counter') }}
              </span>
              <ArrowRightOutlined style="margin-right: 8px; font-size: 10px" />
              <span class="trans-name__txt">HTS</span>
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
  </div>
</template>
<style lang="less">
.kf-market-data__warp {
  height: 100%;
  .trans-name__txt {
    margin-right: 8px;
  }
}
</style>
