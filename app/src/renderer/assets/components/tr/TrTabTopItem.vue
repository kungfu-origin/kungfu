<template>
    <span class="tab-top-item">
        <slot></slot>
        <span class="mouse-over" @click="handleClickItem">
            <slot name="click"></slot>
        </span>
    </span>
    
</template>
<script>
export default {
    name: "tr-tab-top-item",
    methods: {
        handleClickItem(){
            const t = this;
            t.$emit('click')
        }
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.tab-top-item{
    display: inline-block;
    margin-left: 10px;
    margin-right: 6px;
    line-height: $tabHeight;
    height: $tabHeight;
    font-size: 11px;
    vertical-align: bottom;
    .el-select{
        width: 140px;
    }
    .el-input{
        width: 140px;
        .el-input__icon{
            line-height: 26px;
        }
        
    }
    .el-input__inner{
        height: 26px;
        line-height: 26px;
        border-radius: 2px;
        vertical-align:middle;
    }
    .el-date-editor.el-input, .el-date-editor.el-input__inner{
        width:240px;
    }
    .el-range-editor{
        .el-range-separator,.el-range__icon, .el-range__close-icon{
            line-height: 18px;
            font-size: 11px;
        }
    }
    .fa.fa-close{
        font-size: 18px !important;
    }
    
}
</style>
