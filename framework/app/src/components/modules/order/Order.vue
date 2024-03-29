<script setup lang="ts">
import {
  dealKfPrice,
  dealSide,
  dealOffset,
  getIdByKfLocation,
  delayMilliSeconds,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useDownloadHistoryTradingData,
  useTableSearchKeyword,
  useDashboardBodySize,
  confirmModal,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import {
  DownloadOutlined,
  LoadingOutlined,
  CloseOutlined,
  CalendarOutlined,
  PieChartOutlined,
} from '@ant-design/icons-vue';

import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  reactive,
  ref,
  toRaw,
  watch,
} from 'vue';
import { getColumns } from './config';
import {
  dealOrder,
  getKungfuHistoryData,
  kfCancelAllOrders,
  kfCancelOrder,
  makeOrderByOrderInput,
  getOrderLatencyDataByOrderStat,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import { UnfinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  HistoryDateEnum,
  OrderStatusEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  showTradingDataDetail,
  useCurrentGlobalKfLocation,
  useDealDataWithCaches,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import StatisticModal from './OrderStatisticModal.vue';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();

const { processStatusData } = useProcessStatusDetailData();
const { dealDataWithCache, clearCaches } = useDealDataWithCaches<
  KungfuApi.Order,
  KungfuApi.OrderResolvedWithoutStat
>(['uid_key', 'update_time']);
const orders = ref<KungfuApi.OrderResolved[]>([]);
const allOrders = ref<KungfuApi.OrderResolved[]>([]);
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.OrderResolved>(orders, [
    'order_id',
    'instrument_id',
    'exchange_id',
    'source_uname',
    'dest_uname',
  ]);
const unfinishedOrder = ref<boolean>(false);
const historyDate = ref<Dayjs>();
const historyDataLoading = ref<boolean>();

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();
const adjustOrderMaskVisible = ref(false);
const statisticModalVisible = ref<boolean>(false);

const columns = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return getColumns(
      {
        category: 'td',
        group: '*',
        name: '*',
        mode: 'live',
      },
      !!historyDate.value,
    );
  }

  return getColumns(currentGlobalKfLocation.value, !!historyDate.value);
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (historyDate.value) {
          return;
        }

        if (currentGlobalKfLocation.value === null) {
          return;
        }

        if (adjustOrderMaskVisible.value) {
          return;
        }

        const ordersResolved =
          globalThis.HookKeeper.getHooks().dealTradingData.trigger(
            window.watcher,
            currentGlobalKfLocation.value,
            watcher.ledger.Order,
            'order',
          ) as KungfuApi.Order[];

        if (unfinishedOrder.value) {
          const tempAllOrders = ordersResolved.map((item) => {
            return toRaw({
              ...dealDataWithCache(item, () => dealOrder(watcher, item, false)),
              ...getOrderLatencyDataByOrderStat(item, watcher.ledger.OrderStat), // 分离出OrderMedianResolved，解决缓存依赖值变更，但缓存uid_key和update_time不变导致取值错误
            });
          });
          allOrders.value = tempAllOrders;
          orders.value = toRaw(
            tempAllOrders.filter((item) => !isFinishedOrderStatus(item.status)),
          );
          return;
        }

        let finishedOrdersCount = 0;
        const { totalOrders, ordersForTable } = ordersResolved.reduce(
          (preOrders, curOrder) => {
            const orderResolved = toRaw({
              ...dealDataWithCache(curOrder, () =>
                dealOrder(watcher, curOrder, false),
              ),
              ...getOrderLatencyDataByOrderStat(
                curOrder,
                watcher.ledger.OrderStat,
              ),
            });
            preOrders.totalOrders.push(orderResolved);
            if (isFinishedOrderStatus(curOrder.status)) {
              if (finishedOrdersCount < 500) {
                finishedOrdersCount++;
                preOrders.ordersForTable.push(orderResolved);
              }
            } else {
              preOrders.ordersForTable.push(orderResolved);
            }
            return preOrders;
          },
          { totalOrders: [], ordersForTable: [] } as {
            totalOrders: KungfuApi.OrderResolved[];
            ordersForTable: KungfuApi.OrderResolved[];
          },
        );

        allOrders.value = toRaw(totalOrders);
        orders.value = toRaw(ordersForTable);
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(currentGlobalKfLocation, () => {
  historyDate.value = undefined;
  allOrders.value = [];
  orders.value = [];
  clearCaches();
});

watch(historyDate, async (newDate) => {
  clearCaches();
  if (!newDate) {
    return;
  }

  if (currentGlobalKfLocation.value === null) {
    return;
  }

  orders.value = [];
  allOrders.value = [];
  historyDataLoading.value = true;
  delayMilliSeconds(500)
    .then(() =>
      getKungfuHistoryData(
        newDate.format(),
        HistoryDateEnum.naturalDate,
        'Order',
        currentGlobalKfLocation.value as KungfuApi.KfLocation,
      ),
    )
    .then((historyData) => {
      if (!historyData) return;

      const { tradingData } = historyData;

      const orderResolved =
        globalThis.HookKeeper.getHooks().dealTradingData.trigger(
          window.watcher,
          currentGlobalKfLocation.value,
          tradingData.Order,
          'order',
        ) as KungfuApi.Order[];

      const tempAllOrders = toRaw(
        orderResolved.map((item) => {
          return toRaw({
            ...dealDataWithCache(item, () =>
              dealOrder(window.watcher, item, true),
            ),
            ...getOrderLatencyDataByOrderStat(
              item,
              window.watcher.ledger.OrderStat,
            ),
          });
        }),
      );
      allOrders.value = tempAllOrders;
      orders.value = tempAllOrders;
    })
    .catch((err) => {
      if (err.message === 'database_locked') {
        messagePrompt().error(t('export_database_locked'));
      } else {
        console.error(err.message);
      }
    })
    .finally(() => {
      historyDataLoading.value = false;
    });
});

function isFinishedOrderStatus(orderStatus: OrderStatusEnum): boolean {
  return !UnfinishedOrderStatus.includes(orderStatus);
}

function handleCancelOrder(order: KungfuApi.OrderResolved): void {
  if (!currentGlobalKfLocation.value || !window.watcher) {
    error();
    return;
  }

  if (!testOrderSourceIsOnline(order)) {
    return;
  }

  kfCancelOrder(window.watcher, order)
    .then(() => {
      success();
    })
    .catch(() => {
      error();
    });
}

function handleCancelAllOrders(): void {
  if (!currentGlobalKfLocation.value || !window.watcher) {
    error();
    return;
  }
  const name = getIdByKfLocation(currentGlobalKfLocation.value);

  confirmModal(
    t('orderConfig.confirm_cancel_all'),
    `${t('orderConfig.confirm')} ${currentCategoryData.value?.name} ${name} ${t(
      'orderConfig.cancel_all',
    )}`,
  ).then((flag) => {
    if (!flag || !currentGlobalKfLocation.value || !window.watcher) {
      return;
    }

    const orders = getTargetCancelOrders();
    return kfCancelAllOrders(window.watcher, orders)
      .then(() => {
        success();
      })
      .catch((err) => {
        error(err.message);
      });
  });
}

function filterUnfinishedOrders(orders: KungfuApi.Order[]): KungfuApi.Order[] {
  return orders.filter((item) => UnfinishedOrderStatus.includes(item.status));
}

function getTargetCancelOrders(): KungfuApi.Order[] {
  if (!currentGlobalKfLocation.value || !window.watcher) {
    return [];
  }

  return filterUnfinishedOrders(
    globalThis.HookKeeper.getHooks().dealTradingData.trigger(
      window.watcher,
      currentGlobalKfLocation.value,
      window.watcher.ledger.Order,
      'order',
    ) as KungfuApi.Order[],
  );
}

function handleShowTradingDataDetail({
  row,
}: {
  event: MouseEvent;
  row: KungfuApi.TradingDataItem;
}) {
  showTradingDataDetail(
    row as KungfuApi.OrderResolved,
    t('orderConfig.entrust'),
  );
}

const adjustOrderConfig = reactive({
  clientWidth: 0,
  clientHeight: 0,
  offsetLeft: 0,
  offsetTop: 0,
});
const adjustOrderForm = ref<{
  price: number;
  volume: number;
}>({
  price: 0,
  volume: 0,
});
const adjustOrder = ref<KungfuApi.OrderResolved | null>(null);
const tableRef = ref();

function handleAdjustOrder(data: {
  event: MouseEvent;
  row: KungfuApi.TradingDataItem;
  column: KfTradingDataTableHeaderConfig;
}): void {
  const { event, row, column } = data;
  const order = row as KungfuApi.OrderResolved;
  const target = event.target as HTMLElement | null;

  if (column.dataIndex !== 'limit_price') {
    if (column.dataIndex !== 'volume_left') {
      return;
    }
  }

  if (!currentGlobalKfLocation.value || !window.watcher) {
    return;
  }

  if (!testOrderSourceIsOnline(order)) {
    return;
  }

  if (target) {
    adjustOrderMaskVisible.value = true;
    const rectData = target.getBoundingClientRect();
    const tableRectData = tableRef.value.getBoundingClientRect();
    const deltaTop = rectData.top - tableRectData.top;
    adjustOrderConfig.clientWidth = target.clientWidth;
    adjustOrderConfig.clientHeight = target.clientHeight;
    adjustOrderConfig.offsetTop = deltaTop; //header height

    if (column.dataIndex === 'limit_price') {
      adjustOrderConfig.offsetLeft = target.offsetLeft;
    } else {
      adjustOrderConfig.offsetLeft = target.offsetLeft - target.clientWidth;
    }

    adjustOrderForm.value.price = order.limit_price;
    adjustOrderForm.value.volume = +Number(order.volume_left);
    adjustOrder.value = order;
  }
}

function handleClickAdjustOrderMask(): void {
  const kfLocation = currentGlobalKfLocation.value;
  if (!kfLocation) {
    error(t('location_error'));
    adjustOrderMaskVisible.value = false;
    return;
  }

  const order = adjustOrder.value;
  if (!order) {
    adjustOrderMaskVisible.value = false;
    return;
  }

  if (+Number(order.volume_left) === +adjustOrderForm.value.volume) {
    if (+order.limit_price === +adjustOrderForm.value.price) {
      adjustOrderMaskVisible.value = false;
      return;
    }
  }

  kfCancelOrder(window.watcher, order)
    .then(() => {
      const makeOrderInput: KungfuApi.MakeOrderInput = {
        instrument_id: order.instrument_id,
        instrument_type: order.instrument_type,
        exchange_id: order.exchange_id,
        limit_price: +adjustOrderForm.value.price,
        volume: +adjustOrderForm.value.volume,
        price_type: +order.price_type,
        side: +order.side,
        offset: +order.offset,
        hedge_flag: +order.hedge_flag,
        is_swap: !!order.is_swap,
        parent_id: 0n,
      };

      return makeOrderByOrderInput(
        window.watcher,
        makeOrderInput,
        kfLocation,
        getIdByKfLocation(window.watcher.getLocation(order.source)),
      );
    })
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message);
    })
    .finally(() => {
      adjustOrderMaskVisible.value = false;
    });
}

function testOrderSourceIsOnline(order: KungfuApi.OrderResolved) {
  if (!window.watcher) {
    return false;
  }

  const { source, status } = order;
  const tdLocation = window.watcher.getLocation(source);
  const processId = getProcessIdByKfLocation(tdLocation);
  if (processStatusData.value[processId] !== 'online') {
    error(`${t('orderConfig.start')} ${processId} ${t('orderConfig.td')}`);
    return false;
  }

  if (isFinishedOrderStatus(status)) {
    return false;
  }

  return true;
}
</script>
<template>
  <div class="kf-orders__warp kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-checkbox size="small" v-model:checked="unfinishedOrder">
            {{ $t('orderConfig.checkbox_text') }}
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-date-picker
            v-model:value="historyDate"
            :disabled="historyDataLoading"
            :placeholder="$t('orderConfig.date_picker')"
          >
            <template #suffixIcon>
              <LoadingOutlined v-if="historyDataLoading" />
              <CalendarOutlined v-else />
            </template>
          </a-date-picker>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="statisticModalVisible = true">
            <template #icon>
              <PieChartOutlined style="font-size: 14px"></PieChartOutlined>
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            @click="handleDownload('Order', currentGlobalKfLocation)"
          >
            <template #icon>
              <DownloadOutlined style="font-size: 14px" />
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            danger
            @click="handleCancelAllOrders"
          >
            {{ $t('orderConfig.cancel_all') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <div class="kf-table__warp" ref="tableRef">
        <div class="kf-adjust-order-mask__warp" v-if="adjustOrderMaskVisible">
          <div
            class="kf-adjust-order-mask"
            @click.stop="handleClickAdjustOrderMask"
          ></div>
          <a-input-number
            v-if="adjustOrderConfig.clientWidth !== 0"
            class="adjust-order-item price"
            :style="{
              width: adjustOrderConfig.clientWidth + 'px',
              height: adjustOrderConfig.clientHeight + 'px',
              top: adjustOrderConfig.offsetTop + 'px',
              left: adjustOrderConfig.offsetLeft + 'px',
            }"
            v-model:value="adjustOrderForm.price"
          ></a-input-number>
          <a-input-number
            v-if="adjustOrderConfig.clientWidth !== 0"
            class="adjust-order-item order"
            :style="{
              width: adjustOrderConfig.clientWidth + 'px',
              height: adjustOrderConfig.clientHeight + 'px',
              top: adjustOrderConfig.offsetTop + 'px',
              left:
                adjustOrderConfig.offsetLeft +
                adjustOrderConfig.clientWidth +
                'px',
            }"
            v-model:value="adjustOrderForm.volume"
          ></a-input-number>
        </div>
        <KfTradingDataTable
          :columns="columns"
          :dataSource="tableData"
          keyField="uid_key"
          @clickCell="handleAdjustOrder"
          @rightClickRow="handleShowTradingDataDetail"
        >
          <template
            v-slot:default="{
              item,
              column,
            }: {
              item: KungfuApi.OrderResolved,
              column: KfTradingDataTableHeaderConfig,
            }"
          >
            <template v-if="column.dataIndex === 'side'">
              <span :class="`color-${dealSide(item.side).color}`">
                {{ dealSide(item.side).name }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'offset'">
              <span :class="`color-${dealOffset(item.offset).color}`">
                {{ dealOffset(item.offset).name }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'limit_price'">
              {{ dealKfPrice(item.limit_price) }}
            </template>
            <template v-else-if="column.dataIndex === 'volume_left'">
              <span
                style="float: right"
                :title="`${item.volume - item.volume_left} / ${item.volume}`"
              >
                {{ `${item.volume - item.volume_left} / ${item.volume}` }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'status_uname'">
              <span :class="`color-${item.status_color}`">
                {{ item.status_uname }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'source_uname'">
              <span :class="[`color-${item.source_resolved_data.color}`]">
                {{ item.source_uname }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'dest_uname'">
              <span :class="[`color-${item.dest_resolved_data.color}`]">
                {{ item.dest_uname }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'actions'">
              <CloseOutlined
                class="kf-hover"
                @click="handleCancelOrder(item)"
                v-if="!isFinishedOrderStatus(item.status)"
              />
            </template>
          </template>
        </KfTradingDataTable>
      </div>
    </KfDashboard>
    <StatisticModal
      v-if="statisticModalVisible"
      v-model:visible="statisticModalVisible"
      :orders="allOrders"
      :is-unfinished-order="unfinishedOrder"
      :history-date="historyDate"
    ></StatisticModal>
  </div>
</template>
<style lang="less">
.kf-orders__warp {
  width: 100%;
  height: 100%;

  .kf-table__warp {
    width: 100%;
    height: 100%;
    position: relative;

    .kf-trading-data-table {
      width: 100%;
      height: 100%;
    }

    .kf-adjust-order-mask__warp {
      position: absolute;
      width: 100%;
      height: 100%;
      left: 0;
      top: 0;
      z-index: 100;

      .kf-adjust-order-mask {
        position: absolute;
        width: 100%;
        height: 100%;
        left: 0;
        top: 0;
        z-index: 100;
        background: rgba(0, 0, 0, 0.45);
      }

      .adjust-order-item {
        position: absolute;
        z-index: 101;
        background: #141414;

        .ant-input-number-input-wrap {
          height: 100%;

          .ant-input-number-input {
            height: 100%;
          }
        }
      }
    }
  }
}
</style>
