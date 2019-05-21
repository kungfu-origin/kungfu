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
            label="账户名称"
            prop="account_id"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-select v-model.trime="makeOrderForm.account_id">
                    <el-option
                        v-for="account in accountList"
                        :key="account.account_id"
                        :label="account.account_id"
                        :value="account.account_id">
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
                <el-input-number v-model.trim="makeOrderForm.limit_price"></el-input-number>                
            </el-form-item>
            <el-form-item
            label="数量"
            prop="volume"
            class="make-order-volume"
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-input-number v-model.trim="makeOrderForm.volume"></el-input-number>                
            </el-form-item>
            <el-form-item>
                <el-radio-group size="mini" v-model="makeOrderForm.offset">
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
                <el-radio-group size="mini" v-model="makeOrderForm.offset">
                    <el-radio size="mini" :label="0">开</el-radio>
                    <el-radio size="mini" :label="1">平</el-radio>
                    <el-radio size="mini" :label="2">平今</el-radio>
                    <el-radio size="mini" :label="3">平昨</el-radio>
                </el-radio-group>
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer make-order-footer">
            <el-button class="buy" size="default" @click="handleBuy">买 入</el-button>
            <el-button class="sell" size="default" type="primary" @click="handleSell">卖 出</el-button>
        </div>
    </el-dialog>

</template>
<script>
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
        accountList(){},
        accountType(){},
    },

    methods: {
        handleClose(){
            const t = this;
            t.clearData();
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
        
        clearData(){
            const t = this;
            t.$emit('update:visible', false)
            t.form.dateRange = [];
        },
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
    .make-order-dialog{
        .el-input-number{
            width: 200px;
        }
        // .make-order-volume{
        //     .el-form-item__content{
        //         display: flex;
        //         .el-radio-group{
        //             padding-left: 20px;
        //         }
        //     }
        // }
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
