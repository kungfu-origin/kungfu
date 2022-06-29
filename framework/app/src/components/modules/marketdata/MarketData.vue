<script setup lang="ts">
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';

import {
  useDashboardBodySize,
  useTriggerMakeOrder,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import { getColumns } from './config';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
  useQuote,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { StarFilled, PlusOutlined } from '@ant-design/icons-vue';
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
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
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

const { getQuoteByInstrument, getPreClosePrice } = useQuote();
const { customRow, triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();
const { setSubscribedInstruments } = useGlobalStore();

function handleSubscribeAll(): void {
  subscribeAllInstrumentByAppStates(
    processStatusData.value,
    appStates.value,
    mdExtTypeMap.value,
    subscribedInstruments.value,
  );
  success();
}

function handleConfirmAddInstrumentCallback(val: string): Promise<void> {
  const instrumentResolved = transformSearchInstrumentResultToInstrument(val);

  if (!instrumentResolved) {
    return Promise.reject(new Error('标的错误')).catch((err) => {
      error(err.message);
    });
  }

  const targetIndex = subscribedInstruments.value.findIndex((item) => {
    if (item.exchangeId === instrumentResolved.exchangeId) {
      if (item.instrumentId === instrumentResolved.instrumentId) {
        return true;
      }
    }
    return false;
  });

  if (targetIndex !== -1) {
    return Promise.reject(new Error('重复订阅')).catch((err) => {
      error(err.message);
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
      success();
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
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
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
            :placeholder="$t('marketDataConfig.add_market')"
            style="min-width: 140px"
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
          <a-button size="small" @click="handleSubscribeAll">
            {{ $t('marketDataConfig.subscribe_btn') }}
          </a-button>
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
        :emptyText="$t('empty_text')"
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
              <div class="name">
                {{ record.instrumentName || record.instrumentId || '--' }}
              </div>
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
                  :num="getPreClosePrice(record)"
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
