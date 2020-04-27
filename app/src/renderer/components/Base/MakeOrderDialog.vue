<template>
     <el-dialog 
    width="400px" 
    :title="`${moduleType === 'strategy' ? '策略' : '账户'} ${currentId} 下单`"
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClose"
    class="make-order-dialog"
    >
        <el-form ref="make-order-form" label-width="67px" :model="makeOrderForm">
            <el-form-item
            label="代码"
            prop="instrument_id"
            :rules="[
                { required: true, message: '不能为空！'},
            ]">
                <el-autocomplete 
                v-model.trim="makeOrderForm.instrument_id"
                :fetch-suggestions="querySearch"
                ></el-autocomplete>
            </el-form-item>
            <el-form-item
            v-if="moduleType === 'strategy'"
            label="账户"
            prop="name"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
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
            label="类型"
            prop="price_type"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-radio-group size="mini" v-model="makeOrderForm.price_type">
                    <el-radio size="mini" v-for="key in Object.keys(priceType || {})" :label="+key" :key="key">{{ priceType[key] }}</el-radio>
                </el-radio-group>
            </el-form-item>
            <el-form-item
            v-if="makeOrderForm.price_type === 0"
            label="价格"
            prop="limit_price"
            :rules="[
                { required: true, message: '不能为空！' },
                { validator: biggerThanZeroValidator, trigger: 'blur'}
            ]">
                <el-input-number
                :precision="2"
                :step="0.01"
                v-model.trim="makeOrderForm.limit_price"></el-input-number>                
            </el-form-item>
            <el-form-item
            label="数量"
            prop="volume"
            class="make-order-volume"
            :rules="[
                { required: true, message: '不能为空！' },
                { validator: biggerThanZeroValidator, trigger: 'blur'}
            ]">
                <el-input-number 
                :step="100"  
                v-model.trim="makeOrderForm.volume"
                ></el-input-number>                
            </el-form-item>
            <el-form-item
            v-if="isFuture"
            label="开平"
            prop="offset"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-radio-group size="mini" v-model="makeOrderForm.offset">
                    <el-radio size="mini" v-for="key in Object.keys(offsetName || {})" :key="key" :label="+key">{{ offsetName[key] }}</el-radio>
                </el-radio-group>
            </el-form-item>

            <el-form-item
            v-if="isFuture"
            label="套保"
            prop="hedge_flag"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
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
        </el-form>
        <div slot="footer" class="dialog-footer make-order-footer">
            <el-button class="buy" size="medium" type="danger" @click="handleBuy">买 入</el-button>
            <el-button class="sell" size="medium" type="success" @click="handleSell">卖 出</el-button>
        </div>
    </el-dialog>

</template>

<script>
import Vue from 'vue';
import { mapState } from 'vuex';
import { biggerThanZeroValidator } from '__assets/validator';
import { kungfuMakeOrder } from '__io/kungfu/makeCancelOrder';
import { deepClone, ifProcessRunning } from '__gUtils/busiUtils';
import { sourceTypeConfig, offsetName, priceType, hedgeFlag, exchangeIds, instrumentTypes } from '__gConfig/tradingConfig';
import { Autocomplete } from 'element-ui';
import { from } from 'rxjs';

const ls = require('local-storage');

Vue.use(Autocomplete)

export default {
    name: 'make-order-dialog',
    props: {
        visible: {
            type: Boolean,
            default: false
        },

        currentId: {
            type: String,
            default: ''
        },

        moduleType: {
            type: String, //'account' : 'strategy',
            default: ''
        },

        pos: Object

    },

    data(){

        this.sourceTypeConfig = sourceTypeConfig;
        this.offsetName = offsetName;
        this.priceType = priceType;
        this.hedgeFlag = hedgeFlag;
        this.exchangeIds = exchangeIds;

        this.biggerThanZeroValidator = biggerThanZeroValidator;

        return {
            currentAccount: '', //only strategy
            makeOrderForm: {
                category: '',
                group: '', //source_name
                name: '', // account_id
                instrument_id: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
            },
        }
    },

    computed: {
        ...mapState({
            tdAccountSource: state => state.BASE.tdAccountSource,
            strategyList: state => state.STRATEGY.strategyList,
            tdList: state => state.ACCOUNT.tdList,
            accountsAsset: state => state.ACCOUNT.accountsAsset,
            processStatus: state => state.BASE.processStatus,
        }),

        accountType(){
            const sourceName = this.currentSourceName;
            if (!sourceName) return 'stock';
            return this.tdAccountSource[sourceName].typeName
        },

        isFuture () {
            return this.accountType.toLowerCase() === 'future'
        },

        currentAccountResolve () {
            if (this.moduleType === 'account') {
                return this.currentId
            } else if (this.moduleType === 'strategy') {
                return this.currentAccount
            }
        }, 

        currentAccountId() {
            return this.currentAccountResolve.toAccountId()
        },

        currentSourceName() {
            if (this.moduleType === 'account') {
                return this.currentId.toSourceName()
            } else if (this.moduleType === 'strategy') {
                return this.currentAccount.toSourceName();
            }
        }
    },

    methods: {
        handleClose(){
            this.clearData();
        },

        handleBuy(){
            //买：0
            this.makeOrderForm.side = 0;
            this.submit()
        },

        handleSell(){
            //卖：1
            this.makeOrderForm.side = 1;
            this.submit()
        },

        handleSelectAccount(account) {
            this.currentAccount = account;
        },

        submit(){
            const t = this;
            t.$refs['make-order-form'].validate(valid => {
                if(valid) {
                    //需要对account_id再处理
                    let makeOrderForm = deepClone(t.makeOrderForm);
                    const gatewayName = `td_${t.currentSourceName}_${t.currentAccountId}`;

                    if (!ifProcessRunning(gatewayName, t.processStatus)){
                        t.$message.warning(`需要先启动 ${makeOrderForm.name} 交易进程！`)
                        return;
                    }

                    const instrumentType = t.getInstrumentType(t.currentAccountResolve);
                    makeOrderForm['instrument_type'] = instrumentType;

                    //sell
                    if ((t.makeOrderForm.side = 1) && (instrumentType === 'Stock')) {
                        const instrumentId = t.makeOrderForm.instrument_id;
                        const targetVolume = t.makeOrderForm.volume;
                        const posItem = t.pos[instrumentId + '多'] || {};
                        const totalVolume = posItem.totalVolume || 0;
                        if (totalVolume <= targetVolume) {
                            t.$message.warning(`持仓不足！当前 ${instrumentId} 持仓 ${totalVolume}`)
                            return
                        }
                    }

                    if (t.moduleType === 'account') {
                        kungfuMakeOrder(makeOrderForm, t.currentAccountResolve)
                            .then(() => t.$message.success('下单指令已发送！'))
                            .catch(err => t.$message.error(err))
                    } else if (t.moduleType === 'strategy') {
                        kungfuMakeOrder(makeOrderForm, t.currentAccountResolve, t.currentId)
                            .then(() => t.$message.success('下单指令已发送！'))
                            .catch(err => t.$message.error(err))
                    }
                    
                    //save instrumentid to ls
                    const instrumentIdsList = ls.get('instrument_ids_list');
                    ls.set('instrument_ids_list', {
                        ...instrumentIdsList,
                        [makeOrderForm.instrument_id || '']: +new Date().getTime()
                    })
                }
            })
        },

        getInstrumentType (accountId) {
            const sourceName = accountId.split('_')[0] || '';
            const config = this.tdAccountSource[sourceName] || '';
            const typeName = config.typeName || 'Unknow';
            return instrumentTypes[typeName] || 0
        },

        querySearch(queryString, cb) {
            const t = this;
            const instrumentIdsList = ls.get('instrument_ids_list') || {};
            const instrumentIdsListResolve = Object.keys(instrumentIdsList)
                .map(key => ({value: key, insertTime: instrumentIdsList[key]}))
                .sort((a, b) => {
                    if(a.insertTime > b.insertTime) return -1
                    else if(b.insertTime > a.insertTime) return 1
                    else return 0
                })

            const results = (queryString.trim() 
            ? instrumentIdsListResolve.filter(instrumentId => (instrumentId.value.includes(queryString))) 
            : instrumentIdsListResolve)
            cb(results)
        },

        getAvailCash(accountId){
            if(!accountId) return 0;
            const targetAccount = this.accountsAsset[accountId] || null
            if(!targetAccount) return 0
            return targetAccount.avail || 0
        },

        getSourceName(accountId){
            const targetAccount = this.tdList.filter(a => a.account_id.includes(accountId))
            if(!targetAccount.length) return ''
            return targetAccount[0].source_name;
        },

        getAccountType(sourceName){
            return this.tdAccountSource[sourceName]
        },
        
        clearData(){
            this.$emit('update:visible', false)
            this.volumeRate = 0;
            this.currentAccount = '';
            this.makeOrderForm = {
                instrument_id: '',
                account_id: '',
                client_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 3
            };
        },
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";

    .make-order-footer{
        display: flex;
        .el-button{
            width: 50%;
        }
    }
</style>
