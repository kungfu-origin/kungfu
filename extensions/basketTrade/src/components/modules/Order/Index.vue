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
  dealKfTime,
  dealOrder,
  getKungfuHistoryData,
  kfCancelAllOrders,
  kfCancelOrder,
  makeOrderByOrderInput,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import {
  NotTradeAllOrderStatus,
  UnfinishedOrderStatus,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  HistoryDateEnum,
  OrderStatusEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  showTradingDataDetail,
  useCurrentGlobalKfLocation,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import StatisticModal from './OrderStatisticModal.vue';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  useCurrentGlobalBasket,
  useMakeBasketOrderFormModal,
  useMakeOrCancelBasketOrder,
} from '../../../utils/basketTradeUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { BASKET_CATEGORYS } from '../../../config';
import {
  getChaseBasketOrderConfigSettings,
  getReplenishBasketOrderConfigSettings,
} from '../../../config/makeBasketOrderFormConfig';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();

const dataTableRef = ref();
const waittingToSelect = ref<'chase' | 'replenish' | ''>();
const { processStatusData } = useProcessStatusDetailData();
const orders = ref<KungfuApi.OrderResolved[]>([]);
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

const { currentGlobalKfLocation, currentCategoryData } =
  useCurrentGlobalKfLocation(window.watcher);

const {
  currentGlobalBasket,
  currentGlobalBasketOrder,
  currentBasketOrderData,
} = useCurrentGlobalBasket();

const { handleShowMakeBasketOrderModal } = useMakeBasketOrderFormModal();
const {
  chaseBasketOrder,
  replenishBasketOrder,
  getBasketOrderTargetStatusOrdersMap,
} = useMakeOrCancelBasketOrder();

const { handleDownload } = useDownloadHistoryTradingData();
const adjustOrderMaskVisible = ref(false);
const statisticModalVisible = ref<boolean>(false);

const columns = computed(() => {
  if (!currentGlobalBasketOrder.value) {
    return getColumns(
      {
        category: BASKET_CATEGORYS.ORDER,
        group: '*',
        name: '*',
        mode: 'live',
      },
      !!historyDate.value,
    );
  }

  return getColumns(
    currentGlobalBasketOrder.value.basket_order_location,
    !!historyDate.value,
  );
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (historyDate.value) {
          return;
        }

        if (!currentGlobalBasketOrder.value) {
          return;
        }

        if (adjustOrderMaskVisible.value) {
          return;
        }

        const ordersResolved =
          globalThis.HookKeeper.getHooks().dealTradingData.trigger(
            window.watcher,
            currentGlobalBasketOrder.value?.basket_order_location,
            watcher.ledger.Order,
            'order',
          ) as KungfuApi.Order[];

        if (unfinishedOrder.value) {
          orders.value = toRaw(
            ordersResolved
              .slice(0, 2000)
              .filter((item) => !isFinishedOrderStatus(item.status))
              .map((item) =>
                toRaw(dealOrder(watcher, item, watcher.ledger.OrderStat)),
              ),
          );
          return;
        }

        orders.value = toRaw(
          ordersResolved
            .slice(0, 500)
            .map((item) =>
              toRaw(dealOrder(watcher, item, watcher.ledger.OrderStat)),
            ),
        );
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(currentGlobalBasketOrder, () => {
  historyDate.value = undefined;
  orders.value = [];
});

watch(historyDate, async (newDate) => {
  if (!newDate) {
    return;
  }

  if (!currentGlobalBasketOrder.value) {
    return;
  }

  orders.value = [];
  historyDataLoading.value = true;
  delayMilliSeconds(500)
    .then(() =>
      getKungfuHistoryData(
        newDate.format(),
        HistoryDateEnum.naturalDate,
        'Order',
        currentGlobalBasketOrder.value?.basket_order_location,
      ),
    )
    .then((historyData) => {
      if (!historyData) return;

      const { tradingData } = historyData;

      const orderResolved =
        globalThis.HookKeeper.getHooks().dealTradingData.trigger(
          window.watcher,
          currentGlobalBasketOrder.value?.basket_order_location,
          tradingData.Order,
          'order',
        ) as KungfuApi.Order[];

      orders.value = toRaw(
        orderResolved.map((item) =>
          toRaw(dealOrder(window.watcher, item, tradingData.OrderStat, true)),
        ),
      );
      historyDataLoading.value = false;
    })
    .catch((err) => {
      if (err.message === 'database_locked') {
        messagePrompt().error(t('database_locked'));
      } else {
        console.error(err.message);
      }
    });
});

function isFinishedOrderStatus(orderStatus: OrderStatusEnum): boolean {
  return !UnfinishedOrderStatus.includes(orderStatus);
}

function handleCancelOrder(order: KungfuApi.OrderResolved): void {
  if (!currentGlobalBasketOrder.value || !window.watcher) {
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
  if (!currentGlobalBasketOrder.value || !window.watcher) {
    error();
    return;
  }
  const name = getIdByKfLocation(
    currentGlobalBasketOrder.value.basket_order_location,
  );

  confirmModal(
    t('orderConfig.confirm_cancel_all'),
    `${t('orderConfig.confirm')} ${currentCategoryData.value?.name} ${name} ${t(
      'orderConfig.cancel_all',
    )}`,
  ).then((flag) => {
    if (!flag || !currentGlobalBasketOrder.value || !window.watcher) {
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
  if (!currentGlobalBasketOrder.value || !window.watcher) {
    return [];
  }

  return filterUnfinishedOrders(
    globalThis.HookKeeper.getHooks().dealTradingData.trigger(
      window.watcher,
      currentGlobalBasketOrder.value.basket_order_location,
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

  if (!currentGlobalBasketOrder.value || !window.watcher) {
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
  const kfLocation = currentGlobalBasketOrder.value?.basket_order_location;
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

function selectRows(orders: KungfuApi.Order[]) {
  orders.forEach((order) => {
    if (!dataTableRef.value) return;

    dataTableRef.value.handleSelectRow(true, order);
  });
}

function handleChaseOrReplenishClick(type: 'chase' | 'replenish') {
  if (!currentGlobalBasket.value || !currentGlobalBasketOrder.value) return;

  if (!waittingToSelect.value) {
    waittingToSelect.value = type;

    const orders = Object.values(
      getBasketOrderTargetStatusOrdersMap(
        window.watcher,
        currentGlobalBasketOrder.value,
        type === 'chase' ? UnfinishedOrderStatus : NotTradeAllOrderStatus,
      ),
    );
    return selectRows(orders);
  }

  if (type === 'chase') {
    handleShowMakeBasketOrderModal(
      `${t('BasketTrade.chase_order')} ${currentGlobalBasket.value.name}`,
      getChaseBasketOrderConfigSettings(),
      (formState) => {
        currentGlobalBasketOrder.value &&
          handleChaseBasketOrder(currentGlobalBasketOrder.value, formState);
      },
    );
  } else if (type === 'replenish') {
    handleShowMakeBasketOrderModal(
      `${t('BasketTrade.replenish_order')} ${currentGlobalBasket.value.name}`,
      getReplenishBasketOrderConfigSettings(),
      (formState) => {
        currentGlobalBasketOrder.value &&
          handleReplenishBasketOrder(currentGlobalBasketOrder.value, formState);
      },
    );
  }

  waittingToSelect.value = '';
}

function handleChaseBasketOrder(
  basketOrder: KungfuApi.BasketOrderResolved,
  formState,
) {
  if (!currentGlobalBasket.value) return;

  if (!dataTableRef.value) return;

  const orders = Object.values(
    dataTableRef.value.selectedRowsMap,
  ) as unknown as KungfuApi.OrderResolved[];

  return chaseBasketOrder(
    window.watcher,
    currentGlobalBasket.value,
    {
      ...basketOrder,
      price_level: formState.priceLevel,
      price_offset: formState.priceOffset,
    },
    orders,
  );
}

function handleReplenishBasketOrder(
  basketOrder: KungfuApi.BasketOrderResolved,
  formState,
) {
  if (!currentGlobalBasket.value) return Promise.reject();

  if (!dataTableRef.value) return Promise.reject();

  const orders = Object.values(
    dataTableRef.value.selectedRowsMap,
  ) as unknown as KungfuApi.OrderResolved[];

  return replenishBasketOrder(
    window.watcher,
    currentGlobalBasket.value,
    {
      ...basketOrder,
      price_level: formState.priceLevel,
      price_offset: formState.priceOffset,
    },
    orders,
  );
}
</script>
<template>
  <div class="kf-orders__warp kf-translateZ">
    <KfDashboard @board-size-change="handleBodySizeChange">
      <template #title>
        <span v-if="currentGlobalBasketOrder">
          <a-tag
            v-if="currentGlobalBasketOrder"
            :color="currentBasketOrderData?.color || 'default'"
          >
            {{ currentBasketOrderData?.name }}
          </a-tag>
          <span v-if="currentGlobalBasketOrder" class="name">
            {{ dealKfTime(currentGlobalBasketOrder?.insert_time as bigint) }}
          </span>
        </span>
      </template>
      <template #header>
        <KfDashboardItem>
          <a-button
            v-if="waittingToSelect !== 'replenish'"
            :type="waittingToSelect === 'chase' ? 'primary' : 'default'"
            size="small"
            @click.stop="handleChaseOrReplenishClick('chase')"
          >
            {{
              waittingToSelect === 'chase'
                ? $t('BasketTrade.wait_to_confirm', {
                    type: $t('BasketTrade.chase_order'),
                  })
                : $t('BasketTrade.chase_order')
            }}
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem style="margin-right: 16px">
          <a-button
            v-if="waittingToSelect !== 'chase'"
            :type="waittingToSelect === 'replenish' ? 'primary' : 'default'"
            size="small"
            @click.stop="handleChaseOrReplenishClick('replenish')"
          >
            {{
              waittingToSelect === 'replenish'
                ? $t('BasketTrade.wait_to_confirm', {
                    type: $t('BasketTrade.replenish_order'),
                  })
                : $t('BasketTrade.replenish_order')
            }}
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem style="margin-right: 16px">
          <a-button
            v-if="waittingToSelect"
            size="small"
            @click.stop="waittingToSelect = ''"
          >
            {{ $t('BasketTrade.cancel') }}
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-checkbox v-model:checked="unfinishedOrder" size="small">
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
      <div ref="tableRef" class="kf-table__warp">
        <div v-if="adjustOrderMaskVisible" class="kf-adjust-order-mask__warp">
          <div
            class="kf-adjust-order-mask"
            @click.stop="handleClickAdjustOrderMask"
          ></div>
          <a-input-number
            v-if="adjustOrderConfig.clientWidth !== 0"
            v-model:value="adjustOrderForm.price"
            class="adjust-order-item price"
            :style="{
              width: adjustOrderConfig.clientWidth + 'px',
              height: adjustOrderConfig.clientHeight + 'px',
              top: adjustOrderConfig.offsetTop + 'px',
              left: adjustOrderConfig.offsetLeft + 'px',
            }"
          ></a-input-number>
          <a-input-number
            v-if="adjustOrderConfig.clientWidth !== 0"
            v-model:value="adjustOrderForm.volume"
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
          ></a-input-number>
        </div>
        <KfTradingDataTable
          ref="dataTableRef"
          :columns="columns"
          :data-source="tableData"
          key-field="uid_key"
          :selectable="true"
          @click-cell="handleAdjustOrder"
          @right-click-row="handleShowTradingDataDetail"
        >
          <template
            #default="{
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
              {{ item.volume - item.volume_left }} / {{ item.volume }}
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
                v-if="!isFinishedOrderStatus(item.status)"
                class="kf-hover"
                @click="handleCancelOrder(item)"
              />
            </template>
          </template>
        </KfTradingDataTable>
      </div>
    </KfDashboard>
    <StatisticModal
      v-if="statisticModalVisible"
      v-model:visible="statisticModalVisible"
      :orders="tableData"
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
