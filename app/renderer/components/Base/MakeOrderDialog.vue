<template>
     <el-dialog 
    width="400px" 
    :title="`${moduleType === 'strategy' ? '策略' : '账户'} ${currentId} 下单`"
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
                <el-radio-group size="mini" v-model="makeOrderForm.price_type">
                    <el-radio size="mini" :label="0">市价</el-radio>
                    <el-radio size="mini" :label="3">限价</el-radio>
                </el-radio-group>
            </el-form-item>
            <el-form-item
            v-if="makeOrderForm.price_type === 3"
            label="价格"
            prop="limit_price"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
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
                { required: true, message: '不能为空！', trigger: 'blur' },
                { validator: biggerThanZeroValidator, trigger: 'blur'}
            ]">
                <el-input-number 
                :step="100"  
                v-model.trim="makeOrderForm.volume"
                ></el-input-number>                
            </el-form-item>
            <el-form-item
            v-if="accountType === 'future'"
            label="开平"
            prop="offset"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-radio-group size="mini" v-model="makeOrderForm.offset">
                    <el-radio size="mini" :label="0">开</el-radio>
                    <el-radio size="mini" :label="1">平</el-radio>
                    <el-radio size="mini" :label="2">平今</el-radio>
                    <el-radio size="mini" :label="3">平昨</el-radio>
                </el-radio-group>
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer make-order-footer">
            <el-button class="buy" size="medium" type="danger" @click="handleBuy">买 入</el-button>
            <el-button class="sell" size="medium" type="success" @click="handleSell">卖 出</el-button>
        </div>
    </el-dialog>

</template>

<script>
import { mapState } from 'vuex';
import { sourceType } from '@/assets/config/accountConfig'
import { biggerThanZeroValidator } from '@/assets/js/validator';
import { nanoMakeOrder } from '@/io/nano/nanoReq';

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
        // instrument_id; //合约代码
        // account_id; //账号ID
        // client_id; //Client ID
        // limit_price; //价格
        // volume; //数量
        // side; //买卖方向 '0': 买, '1': 卖
        // offset; //开平方向 '0': 开, '1': 平, '2': 平今, '3': 平昨
        // price_type; //价格类型 '0': 市价(任意价), '3': 限价 先提供这两选项吧，别的不一定是交易所通用的
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

            // volumeRate: 0,
            biggerThanZeroValidator
        }
    },

    mounted(){
        const t = this;
        //附值 account_id
        if(t.moduleType === 'account') t.makeOrderForm.account_id = t.getSourceName(t.currentId) + '_' + t.currentId;
    },

    computed: {
        ...mapState({
            strategyList: state => state.STRATEGY.strategyList,
            accountList: state => state.ACCOUNT.accountList,
            accountsAsset: state => state.ACCOUNT.accountsAsset,
            processStatus: state => state.BASE.processStatus
        }),

        accountType(){
            const t = this;
            const targetAccountId = t.makeOrderForm.account_id;
            const targetAccount = t.accountList.filter(a => a.account_id === targetAccountId)
            if(!targetAccount.length) return 'stock'
            const sourceName = targetAccount[0].source_name;
            return t.sourceType[sourceName].typeName
        },
    },

    methods: {
        handleClose(){
            const t = this;
            t.clearData();
        },

        handleBuy(){
            const t = this;
            //买：0
            t.makeOrderForm.side = 0;
            t.submit()
        },

        handleSell(){
            const t = this;
            //卖：1
            t.makeOrderForm.side = 1;
            t.submit()
        },

        submit(){
            const t = this;
            t.$refs['make-order-form'].validate(valid => {
                if(valid) {
                    //需要对account_id再处理
                    const makeOrderForm = t.$utils.deepClone(t.makeOrderForm)
                    const sourceAccountId = makeOrderForm.account_id;
                    const gatewayName = `td_${sourceAccountId}`
                    makeOrderForm.account_id = makeOrderForm.account_id.toAccountId()
                    makeOrderForm.price_type = makeOrderForm.price_type.toString()
                    makeOrderForm.side = makeOrderForm.side.toString()
                    makeOrderForm.offset = makeOrderForm.offset.toString()
                    if((t.processStatus || {})[gatewayName] !== 'online'){
                        t.$message.warning(`需要先启动 ${sourceAccountId} 交易进程！`)
                        return;
                    }
                    t.$message.info('正在发送订单指令...')
                    nanoMakeOrder(gatewayName, makeOrderForm).then(() => t.$message.success('下单指令已发送！'))
                }
            })
        },

        getAvailCash(accountId){
            const t = this;
            if(!accountId) return 0;
            const targetAccount = t.accountsAsset.filter(a => a.accountId === accountId)
            if(!targetAccount.length) return 0
            const cashData = targetAccount[0].cashData || {}
            return cashData.avail || 0
        },

        getSourceName(accountId){
            const t = this;
            const targetAccount = t.accountList.filter(a => a.account_id.indexOf(accountId) !== -1)
            if(!targetAccount.length) return ''
            return targetAccount[0].source_name;
        },

        getAccountType(sourceName){
            return this.sourceType[sourceName]
        },
        
        clearData(){
            const t = this;
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
    .make-order-footer{
        display: flex;
        .el-button{
            width: 50%;
        }

    }
</style>
