<template>
 <el-dialog
        width="450px"
        :visible="visible" 
        :title="`${method === 'add'? '添加' : '设置'} 标的池`"  
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleClose"
        @keyup.enter.native="handleSubmitSetting"
    >
 </el-dialog>
    
</template>

<script>
export default {

    methods: {
        
        handleClose () {}

        handleSubmitSetting () {}
    }

}
</script>

<style>

</style>