<template>
     <el-dialog 
    width="450px" 
    :title="`${moduleType === 'strategy' ? '策略' : '账户'} ${currentId} 下单`"
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    class="make-order-dialog"
    >
        <el-form ref="make-order-form" label-width="52px" :model="makeOrderForm">
            <el-form-item
            label="代码"
            prop="instrument_id"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-input v-model.trim="makeOrderForm.instrument_id"></el-input>
            </el-form-item>
            <el-form-item
            v-if="moduleType === 'strategy'"
            label="账户"
            prop="account_id"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-select v-model.trime="makeOrderForm.account_id">
                    <el-option
                        v-for="account in accountList"
                        :key="account.account_id.toAccountId()"
                        :label="account.account_id.toAccountId()"
                        :value="account.account_id">
                        <span style="color: #fff">{{account.account_id.toAccountId()}}</span>
                        <el-tag :type="getAccountType(account.source_name).type">{{getAccountType(account.source_name).typeName === 'future' ? '期货' : '股票'}}</el-tag>
                        <span style="float: right">可用：{{getAvailCash(account.account_id)}}</span>
                    </el-option>
                </el-select>
            </el-form-item>
            <el-form-item
            v-if="moduleType === 'account'"
            label="策略"
            prop="client_id"
            >
                <el-select v-model.trime="makeOrderForm.client_id" clearable>
                    <el-option
                        v-for="strategy in strategyList"
                        :key="strategy.strategy_id"
                        :label="strategy.strategy_id"
                        :value="strategy.strategy_id">
                    </el-option>
                </el-select>
            </el-form-item>
            <el-form-item
            label="类型"
            prop="price_type"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-select v-model.trime="makeOrderForm.price_type">
                    <el-option label="市价" :value="0"></el-option>
                    <el-option label="限价" :value="3"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item
            v-if="makeOrderForm.price_type === 3"
            label="价格"
            prop="limit_price"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
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
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-input-number 
                :step="100"  
                v-model.trim="makeOrderForm.volume"></el-input-number>                
            </el-form-item>
            <el-form-item v-if="makeOrderForm.price_type === 3">
                <el-radio-group size="mini" v-model="volumeRate" @change="handleChangeVolumeRate">
                    <el-radio size="mini" :label="0">自定义</el-radio>
                    <el-radio size="mini" :label="0.25">1/4</el-radio>
                    <el-radio size="mini" :label="0.5">1/2</el-radio>
                    <el-radio size="mini" :label="1">全部</el-radio>
                </el-radio-group>
            </el-form-item>
            <el-form-item
            v-if="accountType === 'future'"
            label="开平"
            prop="offset"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-select v-model.trime="makeOrderForm.offset">
                    <el-option size="mini" :value="0" label="开"></el-option>
                    <el-option size="mini" :value="1" label="平"></el-option>
                    <el-option size="mini" :value="2" label="平今"></el-option>
                    <el-option size="mini" :value="3" label="平昨"></el-option>
                </el-select>
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer make-order-footer">
            <el-button class="buy" size="default" @click="handleBuy">买 入</el-button>
            <el-button class="sell" size="default" type="primary" @click="handleSell">卖 出</el-button>
        </div>
    </el-dialog>

</template>

<script>
import {mapState} from 'vuex';
import {sourceType} from '@/assets/config/accountConfig'

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

    },

    data(){
    //     {
    //     char instrument_id;   //合约代码
    //     char account_id;         //账号ID
    //     char client_id;           //Client ID
    //     double limit_price;                      //价格
    //     int64_t volume;                          //数量
    //     Side side;                               //买卖方向 '0': 买, '1': 卖
    //     Offset offset;                           //开平方向 '0': 开, '1': 平, '2': 平今, '3': 平昨
    //     PriceType price_type;                    //价格类型 '0': 市价(任意价), '3': 限价 先提供这两选项吧，别的不一定是交易所通用的
    // };
        this.sourceType = sourceType;
        return {
            makeOrderForm: {
                instrument_id: '',
                account_id: '',
                client_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 3
            },

            volumeRate: 0,
        }
    },

    computed: {
        ...mapState({
            strategyList: state => state.STRATEGY.strategyList,
            accountList: state => state.ACCOUNT.accountList,
            accountsAsset: state => state.ACCOUNT.accountsAsset,
        }),

        accountType(){
            const t = this;
            const targetAccountId = t.makeOrderForm.account_id;
            const targetAccount = t.accountList.filter(a => a.account_id === targetAccountId)
            if(!targetAccount.length) return 'stock'
            const sourceName = targetAccount[0].source_name;
            console.log(t.sourceType[sourceName].typeName,' ====')
            return t.sourceType[sourceName].typeName
        },
    },

    methods: {
        handleClose(){
            const t = this;
            t.clearData();
        },

        handleChangeVolumeRate(rate){
            const t = this;
            if(rate === 0) return;
            if(t.makeOrderForm.limit_price === 0) return;
            if(!t.makeOrderForm.account_id) return;
            const avail = getAvailCash(t.makeOrderForm.account_id)
            return Math.floor((avail / t.makeOrderForm.limit_price) * rate)
        },

        handleBuy(){},

        handleSell(){},

        handleSubmitSetting(){
            const t = this;
            t.$refs['make-order-form'].validate(valid => {
                if(valid) {

                }
            })
        },

        getAvailCash(accountId){
            const t = this;
            const targetAccount = t.accountsAsset.filter(a => a.accountId === accountId)
            if(!targetAccount.length) return 0
            const cashData = targetAccount[0].cashData || {}
            return cashData.avail || 0
        },

        getAccountType(sourceName){
            return this.sourceType[sourceName]
        },
        
        clearData(){
            const t = this;
            console.log(1111, t.visible)
            t.$emit('update:visible', false)
            t.volumeRate = 0;
            t.makeOrderForm = {
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
    .make-order-dialog{
        .el-input-number{
            // width: 200px;
        }
    }
    .make-order-footer{
        display: flex;
        .el-button{
            width: 50%;
            background: $green;
        }
        .el-button.buy{
            background: $red;
            color: #fff;
        }
    }
</style>
