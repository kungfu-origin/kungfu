<script setup lang="ts">
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    dealKfNumber,
    dealKfPrice,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useQuote,
    useTriggerOrderBook,
} from '@renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import KfBlinkNum from '@renderer/components/public/KfBlinkNum.vue';

interface OrderBookProps {}
defineProps<OrderBookProps>();

const { currentInstrument } = useTriggerOrderBook();
const { getQuoteByInstrument, getLastPricePercent } = useQuote();

const quoteData = computed(() => {
    if (!currentInstrument.value) {
        return null;
    }

    return getQuoteByInstrument(currentInstrument.value);
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
</script>
<template>
    <div class="kf-order-book__warp">
        <div class="level-book">
            <div
                class="level-row"
                v-for="(item, index) in Array(10)"
                :key="index"
            >
                <div class="buy volume"></div>
                <div class="price">{{ dealKfPrice(askPrices[9 - index]) }}</div>
                <div class="sell volume">
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
                            (
                                ExchangeIds[
                                    currentInstrument?.exchangeId || ''
                                ] || {}
                            ).name || '--'
                        }}
                    </span>
                </div>
            </div>
            <div class="price info-item">
                <div class="main">
                    {{
                        dealKfPrice(
                            getQuoteByInstrument(currentInstrument)?.last_price,
                        )
                    }}
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
            <div
                class="level-row"
                v-for="(item, index) in Array(10)"
                :key="index"
            >
                <div class="buy volume">
                    {{ dealKfNumber(bidVolume[index]) }}
                </div>
                <div class="price">{{ dealKfPrice(bidPrices[index]) }}</div>
                <div class="sell volume"></div>
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
    padding: 0 8px;

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
        height: 60px;
        display: flex;
        align-items: center;

        .info-item {
            flex: 1;
            height: 50px;

            &.info {
                text-align: left;
            }

            &.price {
                text-align: right;
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
