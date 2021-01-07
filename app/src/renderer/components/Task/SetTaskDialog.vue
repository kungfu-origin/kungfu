<template>
     <el-dialog 
    width="540px" 
    title="添加交易任务"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClose"
    @keyup.enter.native="handleSubmitSetting"
    id="set-task-dialog"
    >
        <el-tabs tab-position="left" size="mini" v-model="activeTabName">
            <el-tab-pane
                :lazy="true"
                v-for="(item, index) in configList"
                :key="item.key"
                :label="item.name"
                size="mini"
                :name="index.toString()"
            >
                <ExtConfigForm
                    ref="taskSettingForm"
                    v-model="postFormList[index]"
                    :configList="item.config"
                    method="add"
                ></ExtConfigForm>
            </el-tab-pane>
        </el-tabs>        
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting" id="confirm-set-account-btn">确 定</el-button>
        </div>
     </el-dialog>
</template>

<script>

import ExtConfigForm from '@/components/Base/ExtConfigForm';

export default {

    props: {
        configList: {
            type: Array,
            default: () => []
        },

        visible: {
            type: Boolean,
            default: false
        },

        method: {
            type: String,
            default: 'add'
        },
    },

    components: {

        ExtConfigForm
    },

    data () {

        const postFormList = this.configList.map(c => {});

        return {
            activeTabName: '0',
            postFormList: postFormList
        }
        
    },

    methods: {
        handleClose () {
            this.$emit('update:visible', false);
            this.refreshData();
        },

        handleSubmitSetting () {
            this.$refs.taskSettingForm[this.activeTabName].validate(valid => {
                if (valid) {
                    const postData = this.postFormList[this.activeTabName]
                    this.$emit('confirm', JSON.stringify(postData), this.activeTabName)
                    this.handleClose();
                }
            })
        },

        refreshData () {
            this.postForm = {};
        },
    }
}
</script>

<style lang="scss">
    
</style>