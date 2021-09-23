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

        button {
            box-sizing: border-box;
        }

        .el-checkbox{
            .el-checkbox__label{
                font-size: 12px;
                color: $icon;
            }
        }
        i{
            display: inline-block;
            font-size: 14px;
        }

        .el-select.dashboard-header-selector {
            height: 24px;

            .el-input__inner {
                height: 24px;
                line-height: 24px;
            }

            .el-input__icon {
                line-height: 24px;
            }
        }
    }
</style>

