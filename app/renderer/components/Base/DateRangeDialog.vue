<template>
     <el-dialog 
    width="360px" 
    title="日期范围"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    >
        <el-form ref="date-range-form" label-width="90px" :model="form">
            <el-form-item
            label="日期范围"
            prop="dateRange"
            :rules="[
                { required: true, message: '日期范围不能为空！', trigger: 'blur' },
            ]">
                <el-date-picker
                    v-model.trim="form.dateRange"
                    size="mini"
                    type="daterange"
                    range-separator="～"
                    start-placeholder="开始日期"
                    end-placeholder="结束日期">
                </el-date-picker>      
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
    name: 'date-range-dialog',
    props: {
        visible: {
            type: Boolean,
            default: false
        },
    },

    data(){
        return {
            form: {
                dateRange: []
            }
        }
    },

    methods: {
        handleClose(){
            const t = this;
            t.clearData();
        },

        handleSubmitSetting(){
            const t = this;
            t.$refs['date-range-form'].validate(valid => {
                if(valid) {
                    t.$emit('confirm', t.form.dateRange);
                    t.clearData();
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
