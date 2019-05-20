<template>
    <div :class="{'header-item' : true, 'overflow': width != 'auto'}" :style="{'width': width, 'max-width': maxWidth}"> 
        <slot></slot>
    </div>
</template>
<script>
export default {
    name: 'tr-dashboard-header-item',

    props: {
        width: {
            type: String,
            default: 'auto'
        },
        maxWidth: {
            type: String,
            default: 'auto'
        }
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
    .header-item{
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
        font-size: 12px;
        .el-checkbox{
            .el-checkbox__label{
                font-size: 12px;
                color: $icon;
            }
        }
        i{
            font-size: 14px;
        }
    }
</style>

