<template>
    <el-dialog 
    width="450px" 
    title="手续费设置"  
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    >
        <el-form :model="feeSettingForm" ref="feeSettingForm">
            <el-card v-for="(feeSetting, index) in feeSettingForm.fees" :key="index">
                <div slot="header">
                    <span>{{feeSetting.default ? '默认' : feeSetting.instrumnet_id}}</span>
                    <i class="fa-minus"></i>
                </div>
                <el-form-item 
                v-for="key in Object.keys(feeSetting)" 
                :key="key"
                :label="feeSetting[key].lable"
                :prop="`feeSettingForm.${index}.${key}`"
                >
                    <template v-if="feeSetting[key].type === 'string'">
                        <el-input></el-input>
                    </template>
                    <template v-else-if="feeSetting[key].type === 'number'">
                        <el-input-number></el-input-number>
                    </template>
                    <template v-else-if="feeSetting[key].type === 'select'">
                        <el-select>
                            <el-option v-for="value in feeSetting[key].options" :key="value"></el-option>
                        </el-select>
                    </template>
                    
                </el-form-item>
            </el-card>
        </el-form>
        
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="small">取 消</el-button>
            <el-button type="primary" size="small" @click="handleSubmitSetting">确 定</el-button>
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
    },

    data(){
        this.feeTemplate = feeTemplate;
        //初始化
        let defaultFeeSetting = {
            default: true
        };
        Object.keys(feeTemplate).forEach(key => defaultFeeSetting[key] = '')
        return{
            feeSettingForm: {
                fees: [
                    defaultFeeSetting
                ]
            }
        }
    },

    methods: {
        //关闭窗口的时候清空数据
        handleClose() {
            this.$emit('update:visible', false)
        },


        //确认更改
        handleSubmitSetting(){
            const t = this;
        },

        clearData(){
            const t = this;
        }
    }
}
</script>

