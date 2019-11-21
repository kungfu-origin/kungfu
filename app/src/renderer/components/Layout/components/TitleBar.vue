<template>
<div id="title-bar">{{title}}</div>
</template>    
<script>
export default {
    name: 'title-bar',
    props: {
        title: {
            type: String,
            default: ''
        }
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
#title-bar {
    height: 30px;
    line-height: 30px;
    text-align: center;
    background: $bg_dark;
}
</style>