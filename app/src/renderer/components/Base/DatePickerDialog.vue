<template>
     <el-dialog 
    width="300px" 
    title="选择日期"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClose"
    @keyup.enter.native="handleSubmitSetting"
    >
        <el-form ref="date-range-form" label-width="90px" :model="form">
            <el-form-item
            label="选择日期"
            prop="date"
            :rules="[
                { required: true, message: '日期范围不能为空！', trigger: 'blur' },
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
        return {
            form: {
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
                    this.$emit('confirm', this.form.date);
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
