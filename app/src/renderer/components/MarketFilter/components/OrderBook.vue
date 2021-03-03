<template>
    <tr-dashboard :title="`多档行情 ${instrumentId}`">
        <div class="kf-order-book__warp">
            <div class="ask__warp order-book-part__warp">
                <div class="order-line" @click="handleSelectQuote(askPrices[9 - index], 0)" v-for="(item, index) in new Array(10)" :key="index">
                    <span class="price green">{{ dealNum(askPrices[9 - index]) }}</span>
                    <span class="volume">{{ dealNum(askVolumes[9 - index]) }}</span>
                </div>
            </div>
            <div class="last-price">
                <span class="title">最新价</span>
                <BlinkNum
                    :className="{}"
                    :theKey="`${tickerId}_orderbook`"   
                    :num="lastPrice"
                    >
                </BlinkNum>
            </div> 
            <div class="bid__warp order-book-part__warp">
                <div class="order-line" @click="handleSelectQuote(bidPrices[index], 1)" v-for="(item, index) in new Array(10)" :key="index">
                    <span class="price red">{{ dealNum(bidPrices[index]) }}</span>
                    <span class="volume">{{ dealNum(bidVolumes[index]) }}</span>
                </div>
            </div>
        </div>
    </tr-dashboard>
</template>

<script>

import BlinkNum from './BlinkNum'; 
import { dealQuote } from '__io/kungfu/watcher';

export default {

    props: {

        marketData: {
            type: Object,
            default: () => ({})
        },

        tickerId: {
            type: String,
            default: ''
        },
    },

    components: {
        BlinkNum
    },

    computed: {

        quoteData () {
            const quote = this.marketData[this.tickerId] || null;
            if (quote) {
                return dealQuote(quote)
            } else {
                return quote
            }
        },

        instrumentId () {
            return (this.quoteData || {}).instrumentId || ''
        },

        askPrices () {
            if (!this.quoteData) return [];
            return this.quoteData.askPrices || []
        },

        askVolumes () {
            if (!this.quoteData) return [];
            return this.quoteData.askVolumes || []
        },
        
        bidPrices () {
            if (!this.quoteData) return [];
            return this.quoteData.bidPrices || []
        },

        bidVolumes () {
            if (!this.quoteData) return [];
            return this.quoteData.bidVolumes || []
        },

        lastPrice () {
            if (!this.quoteData) return '--'
            return this.quoteData.lastPrice
        }
    },

    methods: {
        handleSelectQuote (price, side) {
            if (price && this.quoteData) {
                this.$emit('makeOrder', {
                    ...this.quoteData,
                    makeOrderPrice: price,
                    makeOrderSide: side
                })
            }
        },

        dealNum (num) {
            if (num === undefined) {
                return '--'
            }
            else {
                return num
            }
        }
    }

}
</script>

<style lang="scss">

@import "@/assets/scss/skin.scss";

.kf-order-book__warp {
    height: 100%;
    display: flex;
    flex-direction: column;
    padding: 10px 0;
    box-sizing: border-box;

    .last-price {
        height: 48px;
        line-height: 48px;
        font-size: 24px;
        font-weight: 600;
        padding: 0 10px;
        box-sizing: border-box;

        .title {
            font-size: 16px;
        }
    }

    .order-book-part__warp {
        flex: 1;
        display: flex;
        flex-direction: column;
    }

    .price {

        &.red {
            color: $red;
        }

        &.green {
            color: $green;
        }
    }

    .order-line {
        flex: 1;
        display: flex;
        padding: 0 10px;
        box-sizing: border-box;

        &:hover{
            background: $bg_light;
        }

        span {
            flex: 1;
            color: $font;
            height: 100%;
            display: flex;
            text-align: right;
            align-items: center;
            justify-content: flex-end;
            font-family: Consolas, Monaco, monospace,"Microsoft YaHei",sans-serif;
        }
    }

}

</style>