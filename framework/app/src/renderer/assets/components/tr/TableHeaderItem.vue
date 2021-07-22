<template>
    <div class="table-header-item">
        <span class="mouse-over" @click="handleClickItem">
            <slot></slot>
        </span>
        <slot name="no-mouse"></slot>
    </div>
</template>
<script>
export default {
    name: "tr-table-header-item",
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
    .table-header-item{
        float: right;
        font-size: 11px;
        color: $icon;
        margin-left: 24px;
        .el-dropdown-link{
            font-size: 11px;
        }
        .el-select{
            width: 160px;
        }
        .el-input{
            width: 160px;
        }
        .el-date-editor.el-input, .el-date-editor.el-input__inner{
            width:240px;
        }
        i.fa{
            font-size: 11px;
        }
    }
</style>

