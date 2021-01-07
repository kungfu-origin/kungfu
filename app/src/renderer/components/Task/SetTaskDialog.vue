<template>
     <el-dialog 
    width="540px" 
    title="添加交易任务"  
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleCancel"
    @keyup.enter.native="handleSubmitSetting"
    id="set-task-dialog"
    >

        <ExtConfigForm
            ref="taskSettingForm"
            v-model="postForm"
            :configList="configList[0].config"
            method="add"
        ></ExtConfigForm>
        
     </el-dialog>
</template>

<script>

import ExtConfigForm from '@/components/Base/ExtConfigForm';


export default {

    props: {
        configList: {
            type: Array,
            default: () => []
        }
    },

    methods: {
        handleCancel () {},

        handleSubmitSetting () {},
    }
}
</script>

<style>

</style>