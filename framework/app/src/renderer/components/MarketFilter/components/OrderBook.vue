<template>
    <tr-dashboard :title="`行情 ${instrumentId}`">
        <div class="kf-order-book__warp">
            <div class="ask__warp order-book-part__warp">
                <div class="order-line" v-for="(item, index) in new Array(10)" :key="index">
                    <span class="volume buy" @click.stop="handleMakeOrder(askPrices[9 - index], 0)"></span>
                    <span class="price green">{{ dealNum(askPrices[9 - index]) }}</span>
                    <span class="volume sell" @click.stop="handleMakeOrder(askPrices[9 - index], 1)">{{ dealNum(askVolumes[9 - index]) }}</span>
                </div>
            </div>
            <div class="last-price">
                <span class="title">最新价</span>
                <tr-blink-num
                    :className="{}"
                    :theKey="`${tickerId}_orderbook`"   
                    :num="lastPrice"
                    :key="`${tickerId}_orderbook`"
                    >
                </tr-blink-num>
            </div> 
            <div class="bid__warp order-book-part__warp">
                <div class="order-line" v-for="(item, index) in new Array(10)" :key="index">
                    <span class="volume buy" @click.stop="handleMakeOrder(bidPrices[index], 0)">{{ dealNum(bidVolumes[index]) }}</span>
                    <span class="price red">{{ dealNum(bidPrices[index]) }}</span>
                    <span class="volume sell" @click.stop="handleMakeOrder(bidPrices[index], 1)"></span>
                </div>
            </div>
        </div>
    </tr-dashboard>
</template>

<script>
import { toDecimal } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

const MIN_PRICE = 0.001;
const MAX_VOUME = 1000000000;

export default {

    props: {

        marketData: {
            type: Object,
            default: () => ({})
        },

        moduleType: String,

        currentId: String,
    },

    data () {

        return {
            tickerId: "",
        }
    },

    mounted () {
        this.$bus.$on('orderbook-tickerId', ({ instrumentId, exchangeId }) => {
            this.tickerId = `${exchangeId}_${instrumentId}`
        })
    },

    beforeDestroy () {
        this.$bus.$off('orderbook-tickerId')
    },

    computed: {

        quoteData () {
            const quote = this.marketData[this.tickerId] || null;
            return quote;
        },

        instrumentId () {
            return (this.quoteData || {}).instrumentId || ''
        },

        askPrices () {
            if (!this.quoteData) return [];
            const volumes = this.quoteData.askVolumes || [];
            return (this.quoteData.askPrices || [])
                .map((price, index) => {
                    const volume = volumes[index] || 0;
                    if (price < MIN_PRICE || volume > MAX_VOUME || volume <= 0) {
                        return 0;
                    }
                    return price;
                })
        },

        askVolumes () {
            if (!this.quoteData) return [];
            return (this.quoteData.askVolumes || [])
                .map(volume => {
                    if (volume > MAX_VOUME || volume <= 0) {
                        return 0;
                    }
                    return volume;
                })
        },
        
        bidPrices () {
            if (!this.quoteData) return [];
            const volumes = this.quoteData.bidVolumes || [];
            return (this.quoteData.bidPrices || [])
                .map((price, index) => {
                    const volume = volumes[index] || 0;
                    if (price < MIN_PRICE || volume > MAX_VOUME || volume <= 0) {
                        return 0;
                    }
                    return price;
                })        
        },

        bidVolumes () {
            if (!this.quoteData) return [];
            return (this.quoteData.bidVolumes || [])
                .map(volume => {
                    if (volume > MAX_VOUME || volume <= 0) {
                        return 0;
                    }
                    return volume;
                })
        },

        lastPrice () {
            if (!this.quoteData) return '--'
            return this.quoteData.lastPrice
        }
    },

    methods: {
        handleMakeOrder (price, side) {
            this.$bus.$emit('update:make-order', {
                currentId: this.currentId,
                moduleType: this.moduleType,
                orderInput: {
                    ...this.quoteData,
                    side,
                    offset: +side === 0 ? 0 : 1,
                    lastPrice: price,
                    instrumentType: this.quoteData.instrumentTypeOrigin,
                    volume: 0
                }
            })
        },

        dealNum (num) {
            if (num === undefined) {
                return '--'
            }
            else if (+num === 0) {
                return ''
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
        box-sizing: border-box;
        display: flex;

        .title {
            font-size: 16px;
            display: inline-block;
            width: 60px;
            padding-left: 10px;
        }

        span {
            flex: 1;
        }
    }

    .order-book-part__warp {
        flex: 1;
        display: flex;
        flex-direction: column;
    }

    .price {
        box-sizing: border-box;
        padding-right: 10px;

        &.red {
            color: $red;
        }

        &.green {
            color: $green;
        }
    }

    .volume {
        cursor: pointer;
        box-sizing: border-box;

        &.buy {
            background: $red;
            color: $white;
            padding-right: 10px;

            &:hover {
                background: $red1;
            }
        }

        &.sell {
            background: $green;
            color: $white;
            text-align: left !important;
            padding-right: 10px;

             &:hover {
                background: $green2;
            }
        }
    }

    .order-line {
        flex: 1;
        display: flex;
        box-sizing: border-box;

        &:hover{
            background: $bg_light;
        }

        span.volume, span.price {
            display: flex;
            flex: 1;
            height: 100%;
            align-items: center;
            justify-content: flex-end;
            font-family: Consolas, Monaco, monospace,"Microsoft YaHei",sans-serif;
        }

        span.price {
            flex: 2;
        }
    }

}

</style>