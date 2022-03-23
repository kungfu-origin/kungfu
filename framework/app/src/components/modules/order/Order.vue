<script setup lang="ts">
import {
  dealKfPrice,
  dealSide,
  dealOffset,
  dealOrderStatus,
  getIdByKfLocation,
  delayMilliSeconds,
  dealTradingData,
  isTdStrategyCategory,
  getOrderTradeFilterKey,
  dealCategory,
  getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useCurrentGlobalKfLocation,
  useDownloadHistoryTradingData,
  useProcessStatusDetailData,
  useTableSearchKeyword,
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
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import { UnfinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  HistoryDateEnum,
  OrderStatusEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { message, Modal } from 'ant-design-vue';
import { showTradingDataDetail } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';
import StatisticModal from './OrderStatisticModal.vue';

const app = getCurrentInstance();

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

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();
const { getExtraCategoryData } = useExtraCategory();
const adjustOrderMaskVisible = ref(false);
const statisticModalVisible = ref<boolean>(false);

const columns = computed(() => {
  if (currentGlobalKfLocation.data === null) {
    return getColumns('td', !!historyDate.value);
  }

  const { category } = currentGlobalKfLocation.data;
  return getColumns(category, !!historyDate.value);
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (historyDate.value) {
          return;
        }

        if (currentGlobalKfLocation.data === null) {
          return;
        }

        if (adjustOrderMaskVisible.value) {
          return;
        }

        const ordersResolved = isTdStrategyCategory(
          currentGlobalKfLocation.data.category,
        )
          ? ((dealTradingData(
              watcher,
              watcher.ledger,
              'Order',
              currentGlobalKfLocation.data,
            ) || []) as KungfuApi.Order[])
          : (getExtraCategoryData(
              watcher.ledger.Order,
              currentGlobalKfLocation.data,
              'order',
            ) as KungfuApi.Order[]);

        if (unfinishedOrder.value) {
          orders.value = toRaw(
            ordersResolved
              .slice(0, 100)
              .filter((item) => !isFinishedOrderStatus(item.status))
              .map((item) =>
                toRaw(dealOrder(watcher, item, watcher.ledger.OrderStat)),
              ),
          );
          return;
        }

        orders.value = toRaw(
          ordersResolved
            .slice(0, 100)
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

watch(currentGlobalKfLocation, () => {
  historyDate.value = undefined;
  orders.value = [];
});

watch(historyDate, async (newDate) => {
  if (!newDate) {
    return;
  }

  if (currentGlobalKfLocation.data === null) {
    return;
  }

  orders.value = [];
  historyDataLoading.value = true;
  await delayMilliSeconds(500);
  const { tradingData, historyDatas } = await getKungfuHistoryData(
    window.watcher,
    newDate.format(),
    HistoryDateEnum.naturalDate,
    'Order',
    currentGlobalKfLocation.data,
  );
  const orderResolved = isTdStrategyCategory(
    currentGlobalKfLocation.data.category,
  )
    ? toRaw(historyDatas as KungfuApi.Order[])
    : (getExtraCategoryData(
        tradingData.Order,
        currentGlobalKfLocation.data,
        'order',
      ) as KungfuApi.Order[]);
  orders.value = toRaw(
    orderResolved.map((item) =>
      toRaw(dealOrder(window.watcher, item, tradingData.OrderStat, true)),
    ),
  );
  historyDataLoading.value = false;
});

function isFinishedOrderStatus(orderStatus: OrderStatusEnum): boolean {
  return !UnfinishedOrderStatus.includes(orderStatus);
}

function handleCancelOrder(order: KungfuApi.OrderResolved): void {
  if (!currentGlobalKfLocation.data || !window.watcher) {
    message.error('操作失败');
    return;
  }

  if (!testOrderSourceIsOnline(order)) {
    return;
  }

  kfCancelOrderResovled(order, currentGlobalKfLocation.data)
    .then(() => {
      message.success('操作成功');
    })
    .catch(() => {
      message.error('操作失败');
    });
}

function kfCancelOrderResovled(
  order: KungfuApi.OrderResolved,
  kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void> {
  const tdLocation = window.watcher.getLocation(order.source);
  if (kfLocation.category === 'strategy') {
    return kfCancelOrder(
      window.watcher,
      order.order_id,
      tdLocation,
      kfLocation,
    );
  } else {
    return kfCancelOrder(window.watcher, order.order_id, tdLocation);
  }
}

function handleCancelAllOrders(): void {
  if (!currentGlobalKfLocation.data || !window.watcher) {
    message.error('操作失败');
    return;
  }

  const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
    app?.proxy ? app?.proxy.$globalCategoryRegister.getExtraCategory() : {};
  const categoryName = dealCategory(
    currentGlobalKfLocation.data.category,
    extraCategory,
  ).name;
  const name = getIdByKfLocation(currentGlobalKfLocation.data);

  Modal.confirm({
    title: '确认全部撤单',
    content: `确认 ${categoryName} ${name} 全部撤单`,
    okText: '确认',
    cancelText: '取消',
    onOk() {
      if (!currentGlobalKfLocation.data || !window.watcher) {
        return;
      }

      const orders = getTargetCancelOrders();
      return kfCancelAllOrders(
        window.watcher,
        orders,
        currentGlobalKfLocation.data,
      )
        .then(() => {
          message.success('操作成功');
        })
        .catch((err) => {
          message.error(err.message);
        });
    },
  });
}

function getTargetCancelOrders(): KungfuApi.OrderResolved[] {
  if (!currentGlobalKfLocation.data || !window.watcher) {
    return [];
  }
  if (isTdStrategyCategory(currentGlobalKfLocation.data?.category)) {
    const filterKey = getOrderTradeFilterKey(
      currentGlobalKfLocation.data.category,
    );
    return window.watcher.ledger.Order.filter(
      filterKey,
      window.watcher.getLocationUID(currentGlobalKfLocation.data),
    ).list();
  }

  return getExtraCategoryData(
    window.watcher.ledger.Order,
    currentGlobalKfLocation.data,
    'order',
  ) as KungfuApi.OrderResolved[];
}

function handleShowTradingDataDetail({
  row,
}: {
  event: MouseEvent;
  row: TradingDataItem;
}) {
  showTradingDataDetail(row as KungfuApi.OrderResolved, '委托');
}

const adjustOrderConfig = reactive({
  clientWidth: 0,
  clientHeight: 0,
  offsetLeft: 0,
  offsetTop: 0,
});
const adjustOrderForm = ref<{ price: number; volume: number }>({
  price: 0,
  volume: 0,
});
const adjustOrder = ref<KungfuApi.OrderResolved | null>(null);
const tableRef = ref();

function handleAdjustOrder(data: {
  event: MouseEvent;
  row: TradingDataItem;
  column: KfTradingDataTableHeaderConfig;
}): void {
  const { event, row, column } = data;
  const order = row as KungfuApi.OrderResolved;
  const target = event.target as HTMLElement | null;

  if (column.dataIndex !== 'limit_price') {
    if (column.dataIndex !== 'volume_traded') {
      return;
    }
  }

  if (!currentGlobalKfLocation.data || !window.watcher) {
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
  const kfLocation = currentGlobalKfLocation.data;
  if (!kfLocation) {
    message.error('当前 Location 错误');
    adjustOrderMaskVisible.value = false;
    return;
  }

  const order = adjustOrder.value;
  if (!order) {
    adjustOrderMaskVisible.value = false;
    return;
  }

  kfCancelOrderResovled(order, kfLocation)
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
        parent_id: BigInt(order.parent_id),
      };

      return makeOrderByOrderInput(
        window.watcher,
        makeOrderInput,
        kfLocation,
        getIdByKfLocation(window.watcher.getLocation(order.source)),
      );
    })
    .then(() => {
      message.success('操作成功');
    })
    .catch((err) => {
      message.error(err.message);
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
    message.error(`请先启动 ${processId} 交易进程`);
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
    <KfDashboard>
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation.data">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation.data">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation.data) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-checkbox size="small" v-model:checked="unfinishedOrder">
            未完成委托
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            placeholder="关键字"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-date-picker
            v-model:value="historyDate"
            :disabled="historyDataLoading"
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
            @click="handleDownload('Order', currentGlobalKfLocation.data)"
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
            全部撤单
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
            <template v-else-if="column.dataIndex === 'volume_traded'">
              {{ item.volume_traded }} / {{ item.volume }}
            </template>
            <template v-else-if="column.dataIndex === 'status'">
              <span
                :class="`color-${
                  dealOrderStatus(item.status, item.error_msg).color
                }`"
              >
                {{ dealOrderStatus(item.status, item.error_msg).name }}
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
      :orders="tableData"
      :historyDate="historyDate"
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
