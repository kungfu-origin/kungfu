<template>
    <el-dialog 
    width="350px" 
    title="选择交易日"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClose"
    @keyup.enter.native="handleSubmitSetting"
    >
        <el-form ref="date-range-form" label-width="90px" :model="form">
            <el-form-item
            label="日期类型"
            prop="dateType"
            :rules="[
                { required: true, message: '交易日不能为空！', trigger: 'blur' },
            ]"
            >
            <el-radio-group v-model.trim="form.dateType" style="width: 100%">
                <el-row>
                    <el-col :span="24 / dateTypeOptions.length"  v-for="item in dateTypeOptions" :key="item.label">
                        <el-radio :label="item.value">
                            {{item.label}}
                        </el-radio>
                    </el-col>
                </el-row>
            </el-radio-group>
            
            </el-form-item>
            <el-form-item
            label="选择日期"
            prop="date"
            :rules="[
                { required: true, message: '日期不能为空！', trigger: 'blur' },
            ]">
                <el-date-picker
                    v-model.trim="form.date"
                    size="mini"
                    type="date"
                    placeholder="选择日期"
                    >
                </el-date-picker>      
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button :loading="loading" type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
        </div>
    </el-dialog>

</template>
<script>
export default {
    name: 'date-range-dialog',
    props: {
        visible: {
            type: Boolean,
            default: false
        },

        loading: Boolean
    },

    data(){

        this.dateTypeOptions = [{ label: "按自然日", value: 0 }, { label: "按交易日", value: 1 }]

        return {
            form: {
                dateType: 0,
                date: ''
            }
        }
    },

    methods: {
        handleClose(){
            this.clearData();
        },

        handleSubmitSetting(){
            this.$refs['date-range-form'].validate(valid => {
                if(valid) {
                    this.$emit('confirm', {
                        date: this.form.date,
                        dateType: this.form.dateType
                    });
                }
            })
        },
        
        clearData(){
            this.$emit('update:visible', false)
            this.form.date = [];
        },
    }
}
</script>
<style lang="scss" scoped>

</style>
