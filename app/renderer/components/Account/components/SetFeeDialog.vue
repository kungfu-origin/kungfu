<template>
    <el-dialog 
    width="700px" 
    :title="`${accountId.toAccountId()} 手续费设置`"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    >
        <el-form class="fee-setting-form" :model="feeSettingForm" ref="feeSettingForm" label-width="90px" size="mini">
            <el-card v-for="(feeSetting, index) in feeSettingForm.fees" :key="index" size="mini">
                <div slot="header">
                    <span>{{feeSetting.default ? '默认' : (feeSetting.instrument_id || '新费率设置')}}</span>
                    <el-button v-if="accountType !== 'stock'" class="fee-setting-oper" size="mini" @click="handleAddFeeSetting(index)" icon="el-icon-plus" title="添加"></el-button>            
                    <el-button v-if="!feeSetting.default && accountType !== 'stock'" class="fee-setting-oper" size="mini" @click="handleRemoveFeeSetting(index)" icon="el-icon-minus" title="删除"></el-button>            
                </div>
                <el-form-item 
                v-for="(value, key) in feeSetting" 
                v-if="feeTmp[key]"
                :key="key"
                :label="feeTmp[key].label"
                :prop="`fees.${index}.${key}`"
                size="mini"
                :rules="buildRules(feeSetting, key)"
                >
                    <template v-if="feeSetting.default && key === 'instrument_id'"> 默认 </template>
                    <el-input size="mini" v-else-if="feeTmp[key].type === 'string'" :type="feeTmp[key].key" v-model.trim="feeSettingForm.fees[index][key]"></el-input>
                    <el-input-number size="mini" v-else-if="feeTmp[key].type === 'number'"  :controls="false" v-model.trim="feeSettingForm.fees[index][key]"></el-input-number>
                    <el-select size="mini" v-else-if="feeTmp[key].type === 'select'"  collapse-tags v-model="feeSettingForm.fees[index][key]" placeholder="请选择">
                        <el-option
                            v-for="(value, key) in feeTmp[key].options"
                            :key="key"
                            :label="value"
                            :value="key">
                        </el-option>
                    </el-select>
                </el-form-item>
            </el-card>
        </el-form>
        
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
        </div>
    </el-dialog>    
</template>

<script>
import Vue from 'vue';
import {feeTemplate} from '../config/feeConfig.js';
import {Card} from 'element-ui';
Vue.use(Card)

export default {
    props: {
        visible: {
            type: Boolean,
            default: false,
        },
        accountType: {
            type: String,
            default: 'stock'
        },
        accountId: {
            type: String,
            default: ''
        },
        setFeeSettingData: {
            type: Function,
            default: () => {}
        },
        getFeeSettingData: {
            type: Function,
            default: () => {}
        }
    },

    data(){
        const t = this;
        const feeTmp = feeTemplate(t.accountType);
        //初始化
        let defaultFeeSetting = {};
        Object.keys(feeTmp || {}).forEach(key => defaultFeeSetting[key] = '')
        t.defaultFeeSetting = Object.freeze(defaultFeeSetting)
        return {
            feeTmp,
            feeSettingForm: {
                fees: [
                    {...t.defaultFeeSetting, default: true}
                ]
            }
        }
    },

    mounted(){
        const t = this;
        //获取数据
        //需要保证与当前账户类型一致，
        //过滤 stock: 股票 债券 etf, 
        //    future: 其他
        const targetTypes = t.accountType === 'stock' ? [0, 1, 3] : [2, 4]
        t.getFeeSettingData(t.accountId.toAccountId()).then(res => {
            t.feeSettingForm.fees = res.map(res => {
                //指定默认
                if (res.instrument_id === '')  res.default = true
                return res;    
            }).filter(f => targetTypes.indexOf(+f.instrument_type) !== -1)
        })
    },

    methods: {
        //关闭窗口的时候清空数据
        handleClose(){
            const t = this;
            t.clearData();
        },

        //添加费率设置
        handleAddFeeSetting(index){
            const t = this;
            t.feeSettingForm.fees.splice(index + 1, 0, {...t.defaultFeeSetting})

        },

        //删除费率设置
        handleRemoveFeeSetting(index){
            const t = this;
            t.feeSettingForm.fees.splice(index, 1)
        },

        //确认更改
        handleSubmitSetting(){
            const t = this;
            t.$refs['feeSettingForm'].validate(valid => {
                if(valid){
                    const feeSettingData = t.resolveFeeSettingData(t.feeSettingForm.fees, t.sourceType)
                    t.setFeeSettingData(t.accountId.toAccountId(), feeSettingData).then(res => {
                        t.$message.success('操作成功！')
                        t.clearData()
                    })
                    .catch(err => t.$message.error(err || '操作失败！'))
                }
            })
        },

        resolveFeeSettingData(fees, sourceType){
            const instrumentType = sourceType === 'future' ? '2' : '1'
            return fees.map(fee => {
                const f = JSON.parse(JSON.stringify(fee));
                f.default = null;
                delete f.default;
                return {
                    ...f,
                    instrument_type: instrumentType
                }
            })
        },

        buildRules(feeSetting, key){
            const t = this;
            let rules = []
            if(feeSetting.default && key === 'instrument_id'){
                return []
            }else if(!feeSetting.default && key === 'instrument_id'){
                return [
                    ...(t.feeTmp[key].validators || []),
                    {required: true, message: '不能为空！', trigger: 'blur'},
                    {validator: t.repeatInstrumentIdvalidator, trigger: 'blur'},                    
                ]
            }
            else{
                return [
                    ...(t.feeTmp[key].validators || []),
                    {required: true, message: '不能为空！', trigger: 'blur'}]
            }
        },

        repeatInstrumentIdvalidator(rule, value, callback){
            const t = this;
            const existInstrumentIds = t.feeSettingForm.fees.map(f => f.instrument_id).filter(id => !!id)
            if(existInstrumentIds.filter(id => id === value).length >= 2) {
                callback(new Error('已存在！'))
            }else{
                callback()
            }
        },

        clearData(){
            const t = this;
            t.feeSettingForm.fees = [];
            t.$emit('update:visible', false)
        }
    }
}
</script>
<style lang="scss">
.fee-setting-form{
    .fee-setting-oper{
        float: right;
        margin-left: 10px;
    }
    .el-form-item{
        display: inline-block;
        width: 190px;
        margin-bottom: 12px;
        .el-form-item__label{
            font-size: 12px;
        }
    }
}
</style>


