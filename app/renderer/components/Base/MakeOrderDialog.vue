<template>
     <el-dialog 
    width="450px" 
    title="下单"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    >
        <el-form ref="make-order-form" label-width="90px" :model="makeOrderForm">
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
            label="买卖类型"
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
            :rules="[
                { required: true, message: '不能为空！', trigger: 'blur' },
            ]">
                <el-input-number v-model.trim="makeOrderForm.volume"></el-input-number>                
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
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
            }
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
<style lang="scss" scoped>

</style>
