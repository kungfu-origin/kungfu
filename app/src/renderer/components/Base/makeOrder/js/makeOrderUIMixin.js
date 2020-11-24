import Vue from 'vue';
import { biggerThanZeroValidator } from '__assets/validator';
import { deepClone } from '__gUtils/busiUtils';
import { sourceTypeConfig, sideName, offsetName, priceType, hedgeFlag, exchangeIds, instrumentTypes } from '__gConfig/tradingConfig';
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

    template: `
        <tr-dashboard :title="'下单面板' + currentId">
        <div class="kf-make-order-window__body">
            <el-form ref="make-order-form" label-width="60px" :model="makeOrderForm">
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
                            <el-tag :type="getAccountType(account.source_name).type">{{sourceTypeConfig[getAccountType(account.source_name).typeName].name}}</el-tag>
                            <span style="float: right">可用：{{getAvailCash(account.account_id)}}</span>
                        </el-option>
                    </el-select>
                </el-form-item>   
                <el-form-item
                label="代码"
                prop="instrument_id"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'blur'},
                ]">
                    <el-autocomplete 
                    ref="insturment-id-input"
                    v-model.trim="makeOrderForm.instrument_id"
                    :fetch-suggestions="querySearch"
                    placeholder="请输入代码名称"
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
                    { required: true, message: '不能为空！', trigger: 'blur' },
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
                label="买卖"
                prop="side"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'blur' },
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
                    { required: true, message: '不能为空！', trigger: 'blur' },
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
                    { required: true, message: '不能为空！', trigger: 'blur' },
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
                            { required: true, message: '不能为空！', trigger: 'blur' },
                            { validator: biggerThanZeroValidator}
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

                <el-row class="make-order-line">
                    <el-col :span="16">
                        <el-form-item
                        label="数量"
                        prop="volume"
                        class="no-margin"
                        :rules="[
                            { required: true, message: '不能为空！', trigger: 'blur' },
                            { validator: biggerThanZeroValidator}
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
            </el-form>
            <div class="make-order-btns">
                <el-button size="medium" @click="handleMakeOrder">下单</el-button>
            </div>
        </div>
    </tr-dashboard>
    `,


    data () {
        this.sourceTypeConfig = sourceTypeConfig;
        this.offsetName = offsetName;
        this.sideName = sideName;
        this.priceType = filterPriceType(priceType)
        this.hedgeFlag = hedgeFlag;
        this.exchangeIds = exchangeIds;

        this.biggerThanZeroValidator = biggerThanZeroValidator;

        return {
            currentAccount: '', //only strategy
            makeOrderForm: {
                name: '', // account_id in strategy
                instrument_id: '',
                instrument_type: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
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
            if (!sourceName) return 'Stock';
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

        currentSearchTarget () {

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
    },

    methods: {

        handleSelectInstrumentId (item) {
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

        submit () {
            const t = this;
            t.$refs['make-order-form'].validate(valid => {
                if(valid) {
                    //需要对account_id再处理
                    let makeOrderForm = deepClone(t.makeOrderForm);
                    const instrumentType = t.getInstrumentType(t.currentAccountResolved);
                    makeOrderForm['instrument_type'] = instrumentType;

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
            return this.tdAccountSource[sourceName]
        },
        
        clearData () {
            this.$emit('update:visible', false)
            this.currentAccount = '';
            this.makeOrderForm = {
                name: '', // account_id
                instrument_id: '',
                instrument_type: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
            };
        },
    }
}