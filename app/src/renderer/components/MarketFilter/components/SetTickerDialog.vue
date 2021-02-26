<template>
    <el-dialog 
        :title="titleResolved"
        :visible="visible"
        width="440px"
        id="kf-set-ticker-dialog"
        @keyup.enter.native="handleSetTicker"
        @close="handleClose"
        :close-on-click-modal="false"
        :close-on-press-escape="true"
    >
        
    </el-dialog>
</template>

<script>
export default {

    props: {
        visible: {
            type: Boolean,
            default: false
        },

        mode: {
            type: String,
            default: 'add'
        }
    },

    computed: {
        titleResolved () {
            if (this.mode === 'update') {
                return '添加标的'
            } else {
                return '设置标的'
            }
        },
    },

    methods: {

        handleSetTicker () {
            
        },

        handleClose () {},
    },

}
</script>

<style>

</style>