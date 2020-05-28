<!--
 * @Author: your name
 * @Date: 2020-01-15 17:10:34
 * @LastEditTime: 2020-05-28 19:37:50
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/src/renderer/components/Layout/components/TitleBar.vue
--> 
<template>
<div id="title-bar">{{title}}</div>
</template>    
<script>
export default {
    name: 'title-bar',
    props: {
        title: {
            type: String,
            default: '功夫交易系统'
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