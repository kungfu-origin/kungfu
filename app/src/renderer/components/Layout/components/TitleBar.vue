<template>
<div id="title-bar">{{title}}</div>
</template>    
<script>
export default {
    name: 'title-bar',
    props: {
        title: {
            type: String,
            default: 'Kungfu.Trader'
        }
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
#title-bar {
    height: 25px;
    line-height: 25px;
    text-align: center;
    background: $bg_dark;
    color: $input_bg;
}
</style>