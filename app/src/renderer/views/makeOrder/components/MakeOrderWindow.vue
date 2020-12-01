<template>
    <tr-dashboard :title="`下单 ${currentId}`">
        <div class="kf-make-order-window__body">
            <el-form ref="make-order-form" label-width="70px" :model="makeOrderForm">
                <el-form-item
                v-if="moduleType === 'strategy'"
                label="账户"
                prop="name"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change'},
                ]">
                    <el-select v-model.trim="makeOrderForm.name" @change="handleSelectAccount">
                        <el-option
                            v-for="account in tdList"
                            :key="account.account_id.toAccountId()"
                            :label="account.account_id.toAccountId()"
                            :value="account.account_id">
                            <span style="color: #fff">{{account.account_id.toAccountId()}}</span>
                            <el-tag :type="getAccountType(account.source_name).type">{{(sourceTypeConfig[getAccountType(account.source_name).typeName] || {}).name || ''}}</el-tag>
                            <span style="float: right">可用：{{getAvailCash(account.account_id)}}</span>
                        </el-option>
                    </el-select>
                </el-form-item>   

                <el-form-item
                label="代码"
                prop="instrument_id"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'input'},
                ]">
                    <el-autocomplete 
                    ref="insturment-id-input"
                    v-model="makeOrderForm.instrument_id"
                    :fetch-suggestions="querySearch"
                    placeholder="请输入代码名称"
                    @blur="handleBlurInstrumentId"
                    @select="handleSelectInstrumentId"
                    >
                        <template v-slot="{ item }">
                            <div class="make-order-instrument-ids__warp">
                                <div class="make-order-instrument-id-item">
                                    <span class="ticker">{{ item.ticker }}</span>
                                    <span class="name">{{ item.name }}</span>
                                </div>
                                <div class="make-order-instrument-id-item">{{ (item.exchangeId || '').toUpperCase() }}</div>
                            </div>
                        </template>
                    </el-autocomplete>
                </el-form-item>      

                <el-form-item
                label="交易所"
                prop="exchange_id"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]">
                    <el-select v-model.trim="makeOrderForm.exchange_id">
                        <el-option
                            v-for="exchangeId in Object.keys(exchangeIds)"
                            :key="exchangeId"
                            :label="exchangeIds[exchangeId]"
                            :value="exchangeId">
                        </el-option>
                    </el-select>
                </el-form-item>    

                <el-form-item
                v-if="isFuture"
                label="套保投机"
                prop="hedge_flag"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change'},
                ]">
                    <el-select v-model.trim="makeOrderForm.hedge_flag">
                        <el-option
                            v-for="key in Object.keys(hedgeFlag || {})"
                            :key="key"
                            :label="hedgeFlag[key]"
                            :value="+key">
                        </el-option>
                    </el-select>
                </el-form-item>  

                <el-form-item
                label="买卖"
                prop="side"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.side">
                        <el-radio size="mini" :class="{ 'red-radio': +key === 0, 'green-radio': +key === 1 }"  v-for="key in Object.keys(sideName || {}).slice(0, 2)" :key="key" :label="+key">{{ sideName[key] }}</el-radio>
                    </el-radio-group>
                </el-form-item>

                <el-form-item
                v-if="isFuture"
                label="开平"
                prop="offset"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.offset">
                        <el-radio size="mini" :class="{ 'red-radio': +key === 0, 'green-radio': +key !== 0 }" v-for="key in Object.keys(offsetName || {})" :key="key" :label="+key">{{ offsetName[key] }}</el-radio>
                    </el-radio-group>
                </el-form-item>

                <el-form-item
                label="类型"
                prop="price_type"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.price_type">
                        <el-radio size="mini" v-for="key in Object.keys(priceType || {})" :label="+key" :key="key">{{ priceType[key] }}</el-radio>
                    </el-radio-group>
                </el-form-item>


                <el-row class="make-order-line" v-if="makeOrderForm.price_type === 0">
                    <el-col :span="16">
                        <el-form-item
                        label="价格"
                        prop="limit_price"
                        :rules="[
                            { required: true, message: '不能为空！', trigger: 'input' },
                            { validator: biggerThanZeroValidator, trigger: 'blur'}
                        ]">
                            <el-input-number
                            :precision="3"
                            :step="0.001"
                            :controls="false"
                            placeholder="请输入价格"
                            v-model.trim="makeOrderForm.limit_price"></el-input-number>                
                        </el-form-item>
                    </el-col>
                    <el-col :span="8">
                        <div class="make-order-line-info">
                            <span>可用金额</span>
                            <span>{{ avaliableCash || '-' }}</span>
                        </div>
                    </el-col>
                </el-row>

                <el-form-item
                label="买入方式"
                class="no-margin"
                v-if="!isFuture && (makeOrderForm.price_type === 0)"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]"
                >
                    <el-radio-group size="mini" v-model="buyType">
                        <el-radio size="mini"  label="volume">按数量</el-radio>
                        <el-radio size="mini"  label="price">按金额</el-radio>
                    </el-radio-group>
                </el-form-item>

                <el-row class="make-order-line" v-if="buyType === 'volume'">
                    <el-col :span="16">
                        <el-form-item
                        label="数量"
                        prop="volume"
                        class="no-margin"
                        :rules="[
                            { required: true, message: '不能为空！', trigger: 'input' },
                            { validator: biggerThanZeroValidator, trigger: 'blur'}
                        ]">
                            <el-input-number 
                            :step="100"  
                            :controls="false"
                            placeholder="请输入数量"
                            v-model.trim="makeOrderForm.volume"
                            ></el-input-number>                
                        </el-form-item>
                    </el-col>
                    <el-col :span="8">
                        <div class="make-order-line-info">
                            <span>可下单手数</span>
                            <span>{{ avaliableOrderVolume || '-' }}</span>
                        </div>
                    </el-col>
                </el-row>


                <el-row class="make-order-line" v-else>
                    <el-col :span="16">
                        <el-form-item
                        label="总金额"
                        prop="totalPrice"
                        class="no-margin"
                        :rules="[
                            { required: true, message: '不能为空！', trigger: 'input' },
                            { validator: biggerThanZeroValidator, trigger: 'blur'}
                        ]">
                            <el-input-number 
                            :step="100"  
                            :controls="false"
                            placeholder="请输入数量"
                            v-model.trim="makeOrderForm.totalPrice"
                            ></el-input-number>                
                        </el-form-item>
                    </el-col>
                    <el-col :span="8">
                        <div class="make-order-line-info">
                            <span>下单手数</span>
                            <span>{{ avaliableOrderVolume || '-' }}</span>
                        </div>
                    </el-col>
                </el-row>
            </el-form>
            <div class="make-order-btns">
                <el-button size="medium" @click="handleMakeOrder">下单</el-button>
            </div>
        </div>
    </tr-dashboard>
</template>

<script>
import Vue from 'vue';
import { mapState } from 'vuex';
import { biggerThanZeroValidator } from '__assets/validator';
import { deepClone } from '__gUtils/busiUtils';
import { sourceTypeConfig, sideName, offsetName, priceType, hedgeFlag, exchangeIds, instrumentTypes } from '__gConfig/tradingConfig';
import { getFutureTickersConfig, getStockTickersConfig } from '__assets/base'
import { Autocomplete } from 'element-ui';
import { ipcRenderer } from 'electron';

import makeOrderMixin from '@/components/Base/tradingData/js/makeOrderMixin';
import makeOrderIPCMixin from '@/components/Base/tradingData/js/makeOrderIPCMixin';

const ls = require('local-storage');
const REMOTE = require('electron').remote

Vue.use(Autocomplete)

function filterPriceType (priceType) {
    let filterPriceType = {};

    Object.keys(priceType || {}).forEach(key => {
        if (key <= 1) {
            filterPriceType[key] = priceType[key]
        }
    })

    return filterPriceType
}


export default {

    mixins: [ makeOrderMixin, makeOrderIPCMixin ],

    data () {
        this.sourceTypeConfig = sourceTypeConfig;
        this.offsetName = offsetName;
        this.sideName = sideName;
        this.priceType = filterPriceType(priceType)
        this.hedgeFlag = hedgeFlag;
        this.exchangeIds = exchangeIds;

        this.biggerThanZeroValidator = biggerThanZeroValidator;

        return {
            currentId: '',
            moduleType: '',
            makeOrderByPosData: {},

            currentAccount: '', //only strategy
            makeOrderForm: {
                name: '', // account_id in strategy
                instrument_id: '',
                instrument_type: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                totalPrice: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
            },

            buyType: 'volume', // volume or price

            currentSearchTickerList: [],

            futureTickers: [],
            stockTickers: []
        }
    },

    mounted () {
        getFutureTickersConfig()
            .then(res => {
                this.futureTickers = Object.freeze(res)
            });

        getStockTickersConfig()
            .then(res => {
                this.stockTickers = Object.freeze(res)
            })

        this.bindListenRenderEvents();
    },

    computed: {
        ...mapState({
            tdAccountSource: state => state.tdAccountSource,
            strategyList: state => state.strategyList,
            tdList: state => state.tdList,
            accountsAsset: state => state.accountsAsset,
            processStatus: state => state.processStatus,
        }),

        accountType() {
            const sourceName = this.currentSourceName || '';
            if (!sourceName) return 'stock';
            return (this.tdAccountSource[sourceName] || {}).typeName || ''
        },

        isFuture () {
            return (this.accountType || '').toLowerCase() === 'future'
        },

        targetTickersSource () {
            const accountType = (this.accountType || '').toLowerCase();
            if (accountType === 'stock') {
                return this.stockTickers
            } else if (accountType === 'future') {
                return this.futureTickers
            } else {
                return []
            }
        },

        currentAccountResolved () {
            if (this.moduleType === 'account') {
                return this.currentId || ''
            } else if (this.moduleType === 'strategy') {
                return this.currentAccount
            } else {
                return ''
            }
        }, 

        currentAccountId() {
            return this.currentAccountResolved.toAccountId()
        },

        currentSourceName() {
            if (this.moduleType === 'account') {
                return (this.currentId || '').toSourceName()
            } else if (this.moduleType === 'strategy') {
                return this.currentAccount.toSourceName();
            } else {
                return ''
            }
        },

        avaliableCash () {
            if (!this.currentAccountResolved) return ''
            const avaliableCash = this.getAvailCash(this.currentAccountResolved);
            return avaliableCash
        },        

        avaliableOrderVolume () {

            if (this.buyType === 'price') {
                return this.makeOrderForm.volume
            }

            if (this.makeOrderForm.price_type === 0) {
                
                if (this.makeOrderForm.side === 0) { //买
                    
                    const price = +this.makeOrderForm.limit_price;
                    
                    if (!+price) return '';
                    
                    if (!+this.avaliableCash) return ''
                    
                    return Math.floor(this.avaliableCash / price)

                } else if (this.makeOrderForm.side === 1) { //卖
                    
                    const { instrumentId, totalVolume } = this.makeOrderByPosData;

                    if (instrumentId !== this.makeOrderForm.instrument_id) {
                        return ''
                    }

                    return this.makeOrderByPosData.totalVolume || ''
                }                
            } 

            return ''
        },
    },

    watch: {
        makeOrderByPosData (newPosData) {

            if (!Object.keys(newPosData || {}).length) return;

            this.clearData();

            this.$nextTick()
                .then(() => {
                    const { instrumentId, lastPrice, totalVolume, directionOrigin, exchangeId } = newPosData;
                    this.$set(this.makeOrderForm, 'instrument_id', instrumentId);
                    this.$set(this.makeOrderForm, 'exchange_id', exchangeId);
                    this.$set(this.makeOrderForm, 'limit_price', lastPrice);
                    this.$set(this.makeOrderForm, 'volume', totalVolume);
                    
                    if (this.isFuture) {
                        this.$set(this.makeOrderForm, 'offset', 1)
                    }
                    
                    if (directionOrigin === 0) {
                        this.$set(this.makeOrderForm, 'side', 1)
                    } else if (directionOrigin === 1) {
                        this.$set(this.makeOrderForm, 'side', 0)
                    }
                })
        },

        "makeOrderForm.totalPrice" (val) {
            if (this.buyType !== 'price') return;
            const price = +this.makeOrderForm.limit_price;
            const totalPrice = +val;

            if (price && !isNaN(price)) {
                if (totalPrice && !isNaN(totalPrice)) {
                    if (totalPrice > 0) {
                        if (price > 0) {
                            this.$set(this.makeOrderForm, 'volume', Math.floor(totalPrice / price))
                            return;
                        }
                    }
                }
            }

            this.$set(this.makeOrderForm, 'volume', 0)
        },

         "makeOrderForm.limit_price" (val) {
            if (this.buyType !== 'price') return;
            const price = +val;
            const totalPrice = +this.makeOrderForm.totalPrice;

            if (price && !isNaN(price)) {
                if (totalPrice && !isNaN(totalPrice)) {
                    if (totalPrice > 0) {
                        if (price > 0) {
                            this.$set(this.makeOrderForm, 'volume', Math.floor(totalPrice / price))
                            return;
                        }
                    }
                }
            }

            this.$set(this.makeOrderForm, 'volume', 0)
        },

        "makeOrderForm.price_type" (val) {
            if (+val === 1) {
                this.buyType = 'volume'
            }
        }
        
    },

    methods: {

        handleBlurInstrumentId (e, item) {
            const value = e.target.value.trim();
            this.$set(this.makeOrderForm, 'instrument_id', value);

            //当手动输入ticker，清空除了instumentId外的一切
            this.setInstumentIdTimer = setTimeout(() => {
                this.clearData(true)
                clearTimeout(this.setInstumentIdTimer)
            }, 300)
        },

        handleSelectInstrumentId (item) {
            this.setInstumentIdTimer && clearTimeout(this.setInstumentIdTimer)
            this.clearData(true)
            const { ticker, exchangeId } = item;
            this.$set(this.makeOrderForm, 'instrument_id', ticker)
            this.$set(this.makeOrderForm, 'exchange_id', (exchangeId || '').toUpperCase())
            
            this.$nextTick()
                .then(() => {
                    this.$refs['make-order-form'].validateField('instrument_id');
                })
            
        },

        handleClose () {
            this.clearData();
        },

        handleMakeOrder () {
            this.submit()
        },

        handleSelectAccount (account) {
            this.currentAccount = account;
        },

        bindListenRenderEvents () {
            ipcRenderer.on('init-make-order-win-info', (event, info) => {
                const { currentId, makeOrderByPosData, moduleType } = info;                
                this.currentId = currentId;
                this.moduleType = moduleType;
                this.makeOrderByPosData = makeOrderByPosData;
            })
        },

        submit () {
            const t = this;
            t.$refs['make-order-form'].validate(valid => {
                if(valid) {
                    //需要对account_id再处理
                    let makeOrderForm = deepClone(t.makeOrderForm);
                    const instrumentType = t.getInstrumentType(t.currentAccountResolved);
                    makeOrderForm['instrument_type'] = instrumentType;
                    delete makeOrderForm.totalPrice

                    this.makeOrder(this.moduleType, makeOrderForm, t.currentAccountResolved, t.currentId)
                    
                }
            })
        },

        getInstrumentType (accountId) {
            const sourceName = accountId.split('_')[0] || '';
            const config = this.tdAccountSource[sourceName] || '';
            const typeName = config.typeName || 'Unknow';
            return instrumentTypes[typeName] || 0
        },

        querySearch (queryString, cb) {
            const results = this.getSearchTickers(queryString);
            cb(results)
        },

        getSearchTickers (queryString = '') {
            
            return this.targetTickersSource.filter(item => {
                const { ticker, name, exchangeId } = {
                    ticker: '',
                    name: '',
                    exchangeId: '',
                    ...item
                }

                if (ticker.toLowerCase().includes(queryString.toLowerCase())) return true;
                if (name.toLowerCase().includes(queryString.toLowerCase())) return true;
                if (exchangeId.toUpperCase().includes(queryString.toUpperCase())) return true;
                return false
            })
        },

        getAvailCash (accountId) {
            if(!accountId) return 0;
            const targetAccount = this.accountsAsset[accountId] || null
            if(!targetAccount) return 0
            return targetAccount.avail || 0
        },

        getSourceName (accountId) {
            const targetAccount = this.tdList.filter(a => a.account_id.includes(accountId))
            if(!targetAccount.length) return ''
            return targetAccount[0].source_name;
        },

        getAccountType (sourceName) {
            return this.tdAccountSource[sourceName] || {}
        },
        
        clearData (exceptId=false) {
            this.$emit('update:visible', false)
            this.currentAccount = '';
            this.buyType = 'volume';
            
            if (!exceptId) {
                this.$set(this.makeOrderForm, 'instrument_id', '')
                this.$set(this.makeOrderForm, 'name', '')
            }

            this.$set(this.makeOrderForm, 'instrument_type', '')
            this.$set(this.makeOrderForm, 'exchange_id', '')
            this.$set(this.makeOrderForm, 'limit_price', 0)
            this.$set(this.makeOrderForm, 'totalPrice', 0)
            this.$set(this.makeOrderForm, 'volume', 0)
            this.$set(this.makeOrderForm, 'side', 0)
            this.$set(this.makeOrderForm, 'offset', 0)
            this.$set(this.makeOrderForm, 'price_type', 0)
            this.$set(this.makeOrderForm, 'hedge_flag', 0)
 
        },
    }

}
</script>

<style lang="scss">
@import "@/assets/scss/skin.scss";
$size: 25px;
$fontSize: 12px;

.make-order-instrument-ids__warp {
    display: flex;
    justify-content: space-between;
    padding-right: 16px;
    box-sizing: border-box;

    .make-order-instrument-id-item {

        .ticker {
            display: inline-block;
        }
    }
}

.kf-make-order-window__body {
    padding: 0 12px 10px 10px;
    box-sizing: border-box;
    display: flex;
    justify-content: space-around;
    min-height: 100%;

    .el-form {
        flex: 1;

        .el-form-item__error {
            padding-top: 2px;
        }
    }

    .el-form-item {
        margin-bottom: 15px;

        .el-form-item__label {
            font-size: $fontSize;
            text-align: left;    
            padding-right: 5px;
            line-height: 30px;
        }

        .el-form-item__content {
            line-height: 30px;
        }



        &.no-margin {
            margin-bottom: 0px;
        }

        .el-radio {
            margin-right: 8px; 
        }

        .el-radio__label {
            font-size: $fontSize;
            padding-left: 5px;
        }

        .el-radio.is-checked {

            &.green-radio {
                .el-radio__input.is-checked {

                    .el-radio__inner {
                        background: $green;
                        border-color: $green;
                    }
                }
                
                .el-radio__label {
                    color: $green;
                }
            }

            &.red-radio {
                .el-radio__input.is-checked {

                    .el-radio__inner {
                        background: $red;
                        border-color: $red;
                    }
                }
                
                .el-radio__label {
                    color: $red;
                }
            }
        }
    }

    .make-order-line {
        margin-bottom: 15px;

        .make-order-line-info {
            
            span {
                display: block;
                color: $font;
                font-size: 12px;
                text-align: right;
            }
        }
    }

    .make-order-btns {
        padding-left: 20px;
        width: 70px;
        display: flex;
        flex-direction: column;
        justify-content: flex-start;
        box-sizing: border-box;

        .el-button {
            height: 100%;
            width: 100%;
            margin: 0;
            padding: 0 5px;
            box-sizing: border-box;
            text-align: center;
            word-break: break-word;
            word-wrap: unset;
            white-space: normal;
        }
    }
}

</style>