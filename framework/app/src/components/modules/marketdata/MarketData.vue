<script setup lang="ts">
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';

import {
  useDashboardBodySize,
  useExtConfigsRelated,
  useProcessStatusDetailData,
  useTriggerMakeOrder,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import { getColumns } from './config';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  useInstruments,
  useQuote,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { StarFilled, PlusOutlined } from '@ant-design/icons-vue';
import { message } from 'ant-design-vue';
import {
  dealAssetPrice,
  dealKfNumber,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  addSubscribeInstruments,
  removeSubscribeInstruments,
} from '@kungfu-trader/kungfu-js-api/actions';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';

const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
  useDashboardBodySize();

const columns = computed(() => {
  if (dashboardBodyWidth.value < 450) {
    return getColumns(true);
  }
  return getColumns(false);
});

const {
  subscribedInstruments,
  subscribeAllInstrumentByAppStates,
  searchInstrumentResult,
  searchInstrumnetOptions,
  handleSearchInstrument,
  handleConfirmSearchInstrumentResult,
} = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap } = useExtConfigsRelated();

const { getQuoteByInstrument, getLastPricePercent } = useQuote();
const { customRow, triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();
const { setSubscribedInstruments } = useGlobalStore();

function handleSubscribeAll(): void {
  subscribeAllInstrumentByAppStates(
    processStatusData.value,
    appStates.value,
    mdExtTypeMap.value,
    subscribedInstruments.value,
  );
  message.success('操作成功');
}

function handleConfirmAddInstrumentCallback(val: string): Promise<void> {
  const instrumentResolved = transformSearchInstrumentResultToInstrument(val);

  if (!instrumentResolved) {
    return Promise.reject(new Error('标的错误')).catch((err) => {
      message.error(err.message);
    });
  }

  return addSubscribeInstruments([instrumentResolved])
    .then(() => {
      return setSubscribedInstruments();
    })
    .then(() =>
      subscribeAllInstrumentByAppStates(
        processStatusData.value,
        appStates.value,
        mdExtTypeMap.value,
        [instrumentResolved],
      ),
    )
    .then(() => {
      message.success('操作成功');
    });
}

function handleConfirmRemoveInstrument(
  instrument: KungfuApi.InstrumentResolved,
) {
  return removeSubscribeInstruments(instrument)
    .then(() => {
      return setSubscribedInstruments();
    })
    .then(() => {
      message.success('操作成功');
    })
    .catch((err) => {
      message.error(err.message || '操作失败');
    });
}

function triggeOrderBookMakeOrder(instrument: KungfuApi.InstrumentResolved) {
  triggerOrderBook(instrument);
  triggerMakeOrder(instrument, {
    side: SideEnum.Buy,
    offset: OffsetEnum.Open,
    volume: 0,
    price: getQuoteByInstrument(instrument)?.last_price || 0,
  });
}

function handleClickRow(row: KungfuApi.InstrumentResolved) {
  return customRow(row, triggeOrderBookMakeOrder);
}
</script>
<template>
  <div class="kf-market-data__warp">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-select
            show-search
            v-model:value="searchInstrumentResult"
            placeholder="添加自选"
            style="min-width: 100px"
            :filter-option="false"
            :options="searchInstrumnetOptions"
            @search="handleSearchInstrument"
            @change="
              handleConfirmSearchInstrumentResult(
                $event,
                handleConfirmAddInstrumentCallback,
              )
            "
          >
            <template #suffixIcon>
              <PlusOutlined />
            </template>
          </a-select>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleSubscribeAll">订阅</a-button>
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="subscribedInstruments"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :customRow="handleClickRow"
        emptyText="暂无数据"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.InstrumentResolved,
          }"
        >
          <template v-if="column.dataIndex === 'instrumentId'">
            <div class="subscribed-instrument-info">
              <div class="name">{{ record.instrumentName }}</div>
              <div class="code">
                <span>{{ record.instrumentId }}</span>
                <span>
                  {{ ExchangeIds[record.exchangeId].name }}
                </span>
              </div>
            </div>
          </template>
          <template v-else-if="column.dataIndex === 'open_price'">
            {{ dealAssetPrice(getQuoteByInstrument(record)?.open_price) }}
          </template>
          <template v-else-if="column.dataIndex === 'low_price'">
            {{ dealAssetPrice(getQuoteByInstrument(record)?.low_price) }}
          </template>
          <template v-else-if="column.dataIndex === 'high_price'">
            {{ dealAssetPrice(getQuoteByInstrument(record)?.high_price) }}
          </template>

          <template v-else-if="column.dataIndex === 'lastPrice'">
            <div class="last-price-content">
              <div class="price">
                <KfBlinkNum
                  blink-type="color"
                  :num="
                    dealAssetPrice(getQuoteByInstrument(record)?.last_price)
                  "
                ></KfBlinkNum>
              </div>
              <div class="percent">
                <KfBlinkNum
                  blink-type="color"
                  mode="compare-zero"
                  :num="getLastPricePercent(record)"
                ></KfBlinkNum>
              </div>
            </div>
          </template>
          <template v-else-if="column.dataIndex === 'volume'">
            {{ dealKfNumber(getQuoteByInstrument(record)?.volume) }}
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <StarFilled
                style="color: #faad14"
                @click.stop="handleConfirmRemoveInstrument(record)"
              />
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

  .subscribed-instrument-info {
    .name {
      font-size: 12px;
      font-weight: bold;
    }

    .code {
      span {
        font-size: 10px;
        padding-right: 6px;
      }
    }
  }

  .last-price-content {
    .price {
      font-size: 12px;
    }

    .percent {
      font-size: 10px;
    }

    .price,
    .percent {
      position: relative;
      height: 18px;
      width: 100%;

      .kf-blink-num {
        padding: 0;
      }
    }
  }
}
</style>
