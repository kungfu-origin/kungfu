<template>
    <div :class="{'table-header': true, 'mini': 'size' === 'mini', 'no-bg': noBg}">
        <div class="title">
            <slot name="title"></slot>
        </div>
        <slot></slot>
    </div>
    
</template>
<script>
export default {
    name: "tr-table-header",
    props: {
        size: String,
        noBg: {
            type: Boolean,
            default: false
        }
    }
    
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
    .table-header{
        height: $tabHeight;
        line-height: $tabHeight;
        width: 100%;
        padding: 0px 10px;
        box-sizing: border-box;
        overflow: hidden;
        background: $bg_card;
        .title{
            cursor: pointer;
            display: inline-block;
            font-size: 14px;
            color: $title;
            .mouse-over{
                padding-right: 5px;
                font-size: 18px;
            }
        }
        &.no-bg{
            background: none;
        }
    }

    .table-header.mini{
        height: 30px;
        line-height: 30px;
        .title{
            display: inline-block;
            font-size: 14px;
            color: $font;
            
        }
        .table-header-item{
            font-size: 11px;
        }
    }
</style>
