<script setup lang="ts">
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  dealKfNumber,
  dealKfPrice,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useTriggerMakeOrder } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
} from 'vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import { SideEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useQuote } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';

const currentInstrument = ref<KungfuApi.InstrumentResolved | undefined>();
const { getQuoteByInstrument, getLastPricePercent } = useQuote();
const { triggerOrderBookUpdate } = useTriggerMakeOrder();
const app = getCurrentInstance();
const quoteData = computed(() => {
  if (!currentInstrument.value) {
    return null;
  }

  return getQuoteByInstrument(currentInstrument.value);
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$globalBus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'orderbook') {
        currentInstrument.value = data.instrument;
      }
    });

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

const askPrices = computed(() => {
  if (!quoteData.value) {
    return [];
  }

  return quoteData.value.ask_price;
});

const bidPrices = computed(() => {
  if (!quoteData.value) {
    return [];
  }

  return quoteData.value.bid_price;
});

const askVolume = computed(() => {
  if (!quoteData.value) {
    return [];
  }

  return quoteData.value.ask_volume;
});

const bidVolume = computed(() => {
  if (!quoteData.value) {
    return [];
  }

  return quoteData.value.bid_volume;
});

function handleTriggerBuyOrderBookPriceVolume(
  price: number,
  volume: number | bigint,
) {
  if (!currentInstrument.value) {
    return;
  }

  triggerOrderBookUpdate(currentInstrument.value, {
    side: SideEnum.Buy,
    price,
    volume: BigInt(volume),
  });
}

function handleTriggerSellOrderBookPriceVolume(
  price: number,
  volume: number | bigint,
) {
  if (!currentInstrument.value) {
    return;
  }

  triggerOrderBookUpdate(currentInstrument.value, {
    side: SideEnum.Sell,
    price,
    volume: BigInt(volume),
  });
}
</script>
<template>
  <div class="kf-order-book__warp">
    <div class="level-book">
      <div
        class="level-row"
        v-for="(item, index) in Array(10)"
        :key="index"
        @click=""
      >
        <div
          class="buy volume"
          @click="handleTriggerBuyOrderBookPriceVolume(askPrices[9 - index], 0)"
        ></div>
        <div class="price">{{ dealKfPrice(askPrices[9 - index]) }}</div>
        <div
          class="sell volume"
          @click="
            handleTriggerSellOrderBookPriceVolume(
              askPrices[9 - index],
              askVolume[9 - index],
            )
          "
        >
          {{ dealKfNumber(askVolume[9 - index]) }}
        </div>
      </div>
    </div>
    <div class="instrument-info">
      <div class="info info-item">
        <div class="main">
          {{ currentInstrument?.instrumentName || '--' }}
        </div>
        <div class="sub">
          <span>{{ currentInstrument?.instrumentId || '--' }}</span>
          <span>
            {{
              (ExchangeIds[currentInstrument?.exchangeId || ''] || {}).name ||
              '--'
            }}
          </span>
        </div>
      </div>
      <div class="price info-item">
        <div class="main">
          {{ dealKfPrice(getQuoteByInstrument(currentInstrument)?.last_price) }}
        </div>
        <div class="sub">
          <KfBlinkNum
            blink-type="color"
            mode="compare-zero"
            :num="getLastPricePercent(currentInstrument)"
          ></KfBlinkNum>
        </div>
      </div>
    </div>
    <div class="level-book">
      <div class="level-row" v-for="(_item, index) in Array(10)" :key="index">
        <div
          class="buy volume"
          @click="
            handleTriggerBuyOrderBookPriceVolume(
              bidPrices[index],
              bidVolume[index],
            )
          "
        >
          {{ dealKfNumber(bidVolume[index]) }}
        </div>
        <div class="price">{{ dealKfPrice(bidPrices[index]) }}</div>
        <div
          class="sell volume"
          @click="handleTriggerSellOrderBookPriceVolume(bidPrices[index], 0)"
        ></div>
      </div>
    </div>
  </div>
</template>

<style lang="less">
.kf-order-book__warp {
  height: 100%;
  width: 100%;
  display: flex;
  flex-direction: column;
  padding: 8px 8px;
  overflow-y: overlay;

  .level-book {
    flex: 1;
    display: flex;
    flex-direction: column;

    .level-row {
      flex: 1;
      display: flex;
      justify-content: space-between;

      .price,
      .volume {
        flex: 1;
        padding-right: 8px;
        align-items: center;
        font-weight: bold;
        display: flex;
        justify-content: flex-end;
        align-items: center;
      }

      .price {
        flex: 1.5;
      }

      .volume {
        color: #fff;
        cursor: pointer;

        &.buy {
          background: @red-base;

          &:hover {
            background: @red-7;
          }
        }

        &.sell {
          background: @green-base;

          &:hover {
            background: @green-8;
          }
        }
      }
    }
  }

  .instrument-info {
    display: flex;
    align-items: center;
    padding: 8px 0px;

    .info-item {
      flex: 1;

      &.info {
        text-align: left;
        flex: 1.5;
      }

      &.price {
        text-align: right;
        height: 45px;
      }

      .main {
        font-size: 16px;
        font-weight: bold;
      }

      .sub {
        position: relative;
        font-size: 12px;
      }
    }
  }
}
</style>
