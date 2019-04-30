<template>
    <div class="footer-btn">
        <slot></slot>
    </div>
</template>
<script>
export default {
    name: 'tr-footer-item'
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";

.footer-btn{
    width: 110px;
    height: 22px;
    line-height: 24px;
    padding: 0px 10px 0px 10px;
    box-sizing: border-box;
    font-size: 11px; 
    cursor: pointer;
    font-size: 12px;
    i{
        padding-right: 5px;
    }
    .el-popover__reference{
        font-size: 12px;
        i{
            padding-right: 5px;
        }
    }
}
.footer-btn:hover{
    background: $bg_light;
    color: $font;
}

</style>
