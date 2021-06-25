
<template>
    <tr-dashboard :title="`下单 ${currentId || ''}`">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <el-button size="mini" @click="clearData()" style="width: 50px;">重置</el-button>
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
                            <span>
                                <span style="color: #fff">{{account.account_id.toAccountId()}}</span>
                                <el-tag :type="getAccountType(account.source_name).type">{{(SourceTypeConfig[getAccountType(account.source_name).typeName] || {}).name || ''}}</el-tag>
                            </span>
                            <span style="float: right">可用：{{getAvailCash(account.account_id)}}</span>
                        </el-option>
                    </el-select>
                </el-form-item>   

                <el-form-item
                label="标的"
                prop="instrument_id"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'input'},
                ]">
                    <el-autocomplete 
                    ref="insturment-id-input"
                    v-model="makeOrderForm.instrument_id"
                    :fetch-suggestions="querySearch"
                    placeholder="请输入标的"
                    @blur="handleBlurInstrumentId"
                    @select="handleSelectInstrumentId"
                    >
                        <template v-slot="{ item }">
                            <div class="auto-complete-instrument-ids__warp">
                                <div class="auto-complete-instrument-id-item">
                                    <span class="ticker">{{ item.instrument_id }}</span>
                                    <span class="name">{{ item.instrument_name }}</span>
                                </div>
                                <div class="auto-complete-instrument-id-item">{{ (item.exchange_id || '').toUpperCase() }}</div>
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
                            v-for="exchangeId in Object.keys(ExchangeIds)"
                            :key="exchangeId"
                            :label="ExchangeIds[exchangeId]"
                            :value="exchangeId">
                        </el-option>
                    </el-select>
                </el-form-item>  

                <el-form-item
                v-if="allowShorted"
                label="套保"
                prop="hedge_flag"
                class="no-margin"
                :rules="[
                    { required: true, message: '不能为空！', trigger: 'change'},
                ]">
                    <el-radio-group size="mini" v-model="makeOrderForm.hedge_flag">
                        <el-radio size="mini"  v-for="key in Object.keys(HedgeFlag || {})" :key="key" :label="+key">{{ HedgeFlag[key] }}</el-radio>
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
                v-if="allowShorted"
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
                        <el-radio size="mini" v-for="key in Object.keys(PriceType || {})" :label="+key" :key="key">{{ PriceType[key] }}</el-radio>
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
                            <span>{{ avaliableCash || '--' }}</span>
                        </div>
                    </el-col>
                </el-row>

                <el-form-item
                label="方式"
                class="no-margin"
                prop="buyType"
                v-if="!allowShorted && (makeOrderForm.price_type === 0)"
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
                            <span>可下单数量</span>
                            <span>{{ avaliableOrderVolume }}</span>
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
                            <span>{{ avaliableOrderVolume }}</span>
                        </div>
                    </el-col>
                </el-row>
            </el-form>
            <div class="make-order-btns">
                <el-button size="medium" @click="handleMakeOrder">下单</el-button>
                <el-button v-if="rendererType === 'app' && proMode" size="medium" class="make-order-by-task-btn" @click="handleMakeOrderByTask">算法</el-button>
            </div>
        </div>
    </tr-dashboard>
</template>

<script>

import Vue from 'vue';
import { mapGetters } from 'vuex';

import { Autocomplete } from 'element-ui';
import { ipcRenderer } from 'electron';

import { deepClone, delayMiliSeconds } from '__gUtils/busiUtils';
import { SourceTypeConfig, SideName, OffsetName, PriceType, HedgeFlag, ExchangeIds, InstrumentTypes, allowShorted } from 'kungfu-shared/config/tradingConfig';
import { biggerThanZeroValidator } from '__assets/validator';

import instrumentsMixin from '@/assets/mixins/instrumentsMixin';
import makeOrderMixin from '@/components/Base/makeOrder/js/makeOrderMixin';

function getMakeOrderCoreMixin () {
    if (process.env.RENDERER_TYPE === 'makeOrder') {
        return require('@/components/Base/makeOrder/js/makeOrderIPCMixin').default 
    } else {
        return require('@/components/Base/makeOrder/js/makeOrderCoreMixin').default
    }
}

Vue.use(Autocomplete)


export default {

    mixins: [ instrumentsMixin, makeOrderMixin, getMakeOrderCoreMixin() ],
    
    data () {
        this.SourceTypeConfig = SourceTypeConfig;
        this.OffsetName = OffsetName;
        this.SideName = SideName;
        this.PriceType = filterPriceType(PriceType)
        this.HedgeFlag = HedgeFlag;
        this.ExchangeIds = ExchangeIds;

        this.biggerThanZeroValidator = biggerThanZeroValidator;
        this.rendererType = process.env.RENDERER_TYPE

        return {
            
            //from event 
            currentId: "",
            moduleType: "",
            orderInput: {},

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
        }
    },

    mounted () {
        if (process.env.RENDERER_TYPE !== 'makeOrder') {
            this.$bus.$on('update:make-order', makeOrderData => {
                const { currentId, moduleType, orderInput } = makeOrderData || {};
                this.orderInput = orderInput
                this.currentId = currentId;
                this.moduleType = moduleType
            })
        } else {
            ipcRenderer.on('init-make-order-win-info', (event, info) => {
                const { currentId, orderInput, moduleType } = info;                
                this.currentId = currentId;
                this.moduleType = moduleType;
                this.orderInput = orderInput;
            })
        }

    },

    beforeDestroy () {
        if (process.env.RENDERER_TYPE !== 'makeOrder') {
            this.$bus.$off('update:make-order')
        }
    },

    computed: {


        ...mapGetters([
            "proMode"
        ]),

        accountType() {
            const sourceName = this.currentSourceName || '';
            if (!sourceName) return 'stock';
            return (this.tdAccountSource[sourceName] || {}).typeName || ''
        },

        allowShorted () {
            const { instrumentType } = this.orderInput || {};
            return allowShorted(instrumentType)
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
                return this.makeOrderForm.volume || 0
            }

            if (this.makeOrderForm.price_type === 0) {
                if (this.makeOrderForm.side === 0) { //买
                    const price = +this.makeOrderForm.limit_price;
                    if (!+price) return 0;
                    if (!+this.avaliableCash) return 0
                    if (this.avaliableCash < 0) return 0
                    return Math.floor(this.avaliableCash / price)
                } else if (this.makeOrderForm.side === 1) { //卖
                    const { instrumentId, totalVolume } = this.orderInput;
                    if (instrumentId !== this.makeOrderForm.instrument_id) {
                        return 0
                    }
                    return (totalVolume < 0 ? 0 : totalVolume) || ''
                }                
            } 

            return 0
        },
    },

    watch: {

        orderInput (newOrderInput) {
            
            if (!Object.keys(newOrderInput || {}).length) return;
            
            this.clearData(true);

            const { instrumentId, lastPrice, totalVolume, directionOrigin, side, exchangeId, accountIdResolved, instrumentType } = newOrderInput;

            this.$set(this.makeOrderForm, 'instrument_id', instrumentId);
            this.$set(this.makeOrderForm, 'exchange_id', exchangeId);
            this.$set(this.makeOrderForm, 'limit_price', lastPrice);
            this.$set(this.makeOrderForm, 'volume', totalVolume);
            this.$set(this.makeOrderForm, 'instrument_type', instrumentType);

            if (this.moduleType !== 'strategy') {
                this.$set(this.makeOrderForm, 'name', accountIdResolved);
                this.currentAccount = accountIdResolved
            }
            
            if (directionOrigin === 0) {
                this.$set(this.makeOrderForm, 'side', 1);
                this.$set(this.makeOrderForm, 'offset', 0);
            } else if (directionOrigin === 1) {
                this.$set(this.makeOrderForm, 'side', 0);
                this.$set(this.makeOrderForm, 'offset', 0);
            }

            if (side !== undefined && !Number.isNaN(+side)) {
                this.$set(this.makeOrderForm, 'side', +side);
            }

            delayMiliSeconds(300)
                .then(() => {
                    this.$refs['make-order-form'].clearValidate();
                    this.$refs['make-order-form'].validate()
                        .catch(() => {
                            this.$nextTick()
                                .then(() => {
                                    const $errordoms = document.querySelectorAll('.kf-make-order-window__body .el-form-item__error');
                                    if ($errordoms.length) {
                                        const $errorDom = $errordoms[0];
                                        const $paDom = $errorDom.parentElement;
                                        const $input = $paDom.getElementsByTagName('input');
                                        if ($input.length) {
                                            $input[0].focus();
                                        }
                                    }
                            })
                        })
                })
            
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

        handleMakeOrderByTask () {
            this.$bus.$emit('set-task', {
                type: "trade", 
                initData: {
                    'exchangeId': this.makeOrderForm.exchange_id,
                    'instrumentId': this.makeOrderForm.instrument_id,
                    'account': this.currentAccountResolved
                }
            })
        },

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
            const { instrument_id, exchange_id } = item;
            this.$set(this.makeOrderForm, 'instrument_id', instrument_id)
            this.$set(this.makeOrderForm, 'exchange_id', (exchange_id || '').toUpperCase())
            
            this.$nextTick()
                .then(() => {
                    this.$refs['make-order-form'].validateField('instrument_id');
                })
            
        },

        handleClose () {
            this.clearData();
        },

        handleMakeOrder () {
            this.$refs['make-order-form'].clearValidate();
            this.$refs['make-order-form'].validate(valid => {
                if(valid) {

                    //当下单不是从posdata进入
                    if (!this.makeOrderForm.instrument_type) {
                        const instrumentType = this.getInstrumentType(this.currentAccountResolved);
                        this.$set(this.makeOrderForm, 'instrument_type', instrumentType)
                    }

                    const makeOrderForm = deepClone(this.makeOrderForm);
                    const makeOrderConfirmTip = this.buildMakeOrderFormInfo(makeOrderForm)
                    this.$confirm(makeOrderConfirmTip, '提示', {
                        confirmButtonText: '确 定',
                        cancelButtonText: '取 消',
                    })
                        .then(() => this.makeOrder(this.moduleType, makeOrderForm, this.currentAccountResolved, this.currentId))
                        .then(() => this.$message.success('下单指令已发送！'))
                        .catch(err => {
                            if (err === 'cancel') return;    
                            this.$message.error(err.message || '下单指令发送失败！')
                        })
                }
            })
        },

        handleSelectAccount (account) {
            this.currentAccount = account;
        },

        buildMakeOrderFormInfo (makeOrderForm) {
            const { side, offset, volume, limit_price, accountId, instrument_id, price_type } = makeOrderForm;

            const sideName = SideName[side];
            const offsetName = OffsetName[offset];
            const priceType = PriceType[price_type];

            const tips = `确认下单
            标的 ${instrument_id},
            ${sideName},
            ${offsetName},
            ${priceType},
            价格 ${limit_price},
            数量 ${volume},
            下单账户 ${accountId || this.currentAccountResolved}
            `

            return tips
        },

        getInstrumentType (accountId) {
            const sourceName = accountId.split('_')[0] || '';
            const config = this.tdAccountSource[sourceName] || '';
            const typeName = config.typeName || 'Unknow';
            return InstrumentTypes[typeName] || 0
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
            this.$emit('update:visible', false);

            if (!exceptId) {
                this.$refs['make-order-form'].resetFields();
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

            
            this.$refs['make-order-form'].clearValidate();
        },
    }
}

function filterPriceType (PriceType) {
    let filterPriceType = {};

    Object.keys(PriceType || {}).forEach(key => {
        if (key <= 1) {
            filterPriceType[key] = PriceType[key]
        }
    })

    return filterPriceType
}


</script>

