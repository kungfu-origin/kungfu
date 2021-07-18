<template>
    <div class="tab-top">
        <slot></slot>
    </div>
    
</template>
<script>
export default {
    name: "tr-tab-top",
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.tab-top{
    position: absolute;
    top: -37px;
    right: 0px;
    height: $tabHeight;
    line-height: $tabHeight;
}
</style>
