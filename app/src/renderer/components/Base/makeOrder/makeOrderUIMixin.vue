
<template>
    <tr-dashboard :title="`下单 ${currentId}`">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <el-button size="mini" @click="clearData" style="width: 50px;">重置</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="kf-make-order-window__body">
            <el-form ref="make-order-form" label-width="60px" :model="makeOrderForm">
                <el-form-item
                v-if="moduleType !== 'account'"
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

                <el-row class="make-order-line">
                    <el-col :span="14">
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
                    </el-col>
                    <el-col :span="1">
                        <div :style="{ width: '1px', height: '1px' }"></div>
                    </el-col>
                    <el-col  :span="9">
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
                    </el-col>
                </el-row>

                <el-form-item
                v-if="isFuture"
                label="套保"
                prop="hedge_flag"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change'},
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.hedge_flag">
                        <el-radio size="mini"  v-for="key in Object.keys(hedgeFlag || {})" :key="key" :label="+key">{{ hedgeFlag[key] }}</el-radio>
                    </el-radio-group>
                </el-form-item>  

                <el-form-item
                label="买卖"
                prop="side"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.side">
                        <el-radio size="mini" :class="{ 'red-radio': +key === 0, 'green-radio': +key === 1 }"  v-for="key in Object.keys(SideName || {}).slice(0, 2)" :key="key" :label="+key">{{ SideName[key] }}</el-radio>
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
                        <el-radio size="mini" :class="{ 'red-radio': +key === 0, 'green-radio': +key !== 0 }" v-for="key in Object.keys(OffsetName || {})" :key="key" :label="+key">{{ OffsetName[key] }}</el-radio>
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
                    <el-col :span="14">
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
                    <el-col :span="10">
                        <div class="make-order-line-info">
                            <span>可用金额</span>
                            <span>{{ avaliableCash || '-' }}</span>
                        </div>
                    </el-col>
                </el-row>

                <el-form-item
                label="方式"
                class="no-margin"
                prop="buyType"
                v-if="!isFuture && (makeOrderForm.price_type === 0)"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change' },
                ]"
                >
                    <el-radio-group size="mini" v-model="makeOrderForm.buyType">
                        <el-radio size="mini"  label="volume">按数量</el-radio>
                        <el-radio size="mini"  label="price">按金额</el-radio>
                    </el-radio-group>
                </el-form-item>

                <el-row v-if="makeOrderForm.buyType === 'volume'">
                    <el-col :span="14">
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
                    <el-col :span="10">
                        <div class="make-order-line-info">
                            <span>可下单手数</span>
                            <span>{{ avaliableOrderVolume || '-' }}</span>
                        </div>
                    </el-col>
                </el-row>


                <el-row v-else>
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
import { biggerThanZeroValidator } from '__assets/validator';
import { deepClone } from '__gUtils/busiUtils';
import { sourceTypeConfig, SideName, OffsetName, priceType, hedgeFlag, exchangeIds, InstrumentTypes } from '__gConfig/tradingConfig';
import { getFutureTickersConfig, getStockTickersConfig } from '__assets/base'
import { Autocomplete } from 'element-ui';

const ls = require('local-storage');

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
    
    data () {
        this.sourceTypeConfig = sourceTypeConfig;
        this.OffsetName = OffsetName;
        this.SideName = SideName;
        this.priceType = filterPriceType(priceType)
        this.hedgeFlag = hedgeFlag;
        this.exchangeIds = exchangeIds;

        this.biggerThanZeroValidator = biggerThanZeroValidator;

        return {

            currentAccount: '', //except account
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
                buyType: 'volume', // volume or price
            },

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

        this.init();
    },

    computed: {

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
            } else if (this.moduleType === 'ticker') {
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
            } else if (this.moduleType === 'ticker') {
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

            if (this.makeOrderForm.buyType === 'price') {
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
            this.clearData(true);

            const { instrumentId, lastPrice, totalVolume, directionOrigin, exchangeId, accountIdResolved, instrumentType } = newPosData;

            this.$set(this.makeOrderForm, 'instrument_id', instrumentId);
            this.$set(this.makeOrderForm, 'exchange_id', exchangeId);
            this.$set(this.makeOrderForm, 'limit_price', lastPrice);
            this.$set(this.makeOrderForm, 'volume', totalVolume);
            this.$set(this.makeOrderForm, 'instrument_type', instrumentType);

            if (this.moduleType !== 'strategy') {
                this.$set(this.makeOrderForm, 'name', accountIdResolved);
                this.currentAccount = accountIdResolved
            }
            
            if (this.isFuture) {
                this.$set(this.makeOrderForm, 'offset', 1)
            } else {
                this.$set(this.makeOrderForm, 'offset', 0)
            }
            
            if (directionOrigin === 0) {
                this.$set(this.makeOrderForm, 'side', 1)
            } else if (directionOrigin === 1) {
                this.$set(this.makeOrderForm, 'side', 0)
            }
        },

        'makeOrderForm.buyType' (val) {
            if (val === 'price') {
                this.$set(this.makeOrderForm, 'volume', 0)
            }
        },

        "makeOrderForm.totalPrice" (val) {
            if (this.makeOrderForm.buyType !== 'price') return;
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
            if (this.makeOrderForm.buyType !== 'price') return;
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
                this.makeOrderForm.buyType = 'volume'
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
            this.$refs['make-order-form'].validate(valid => {
                
                if(valid) {
                    //当下单不是从posdata进入
                    if (!this.makeOrderForm.instrument_type) {
                        const instrumentType = this.getInstrumentType(this.currentAccountResolved);
                        this.$set(this.makeOrderForm, 'instrument_type', instrumentType)
                    }

                    let makeOrderForm = deepClone(this.makeOrderForm);
                    this.makeOrder(this.moduleType, makeOrderForm, this.currentAccountResolved, this.currentId)
                        .then(() => this.$message.success('下单指令已发送！'))
                        .catch(err => this.$message.error(err.message || '下单指令发送失败！'))
                }
            })
        },

        handleSelectAccount (account) {
            this.currentAccount = account;
        },

        getInstrumentType (accountId) {
            const sourceName = accountId.split('_')[0] || '';
            const config = this.tdAccountSource[sourceName] || '';
            const typeName = config.typeName || 'Unknow';
            return InstrumentTypes[typeName] || 0
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
            this.$refs['make-order-form'].resetFields();
            this.$emit('update:visible', false)
            
            if (!exceptId) {
                this.$set(this.makeOrderForm, 'instrument_id', '')
                this.$set(this.makeOrderForm, 'name', '')
                this.currentAccount = '';
            }

            this.$set(this.makeOrderForm, 'buyType', 'volume')
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