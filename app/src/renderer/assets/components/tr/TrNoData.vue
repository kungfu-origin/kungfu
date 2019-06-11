<template>
    <div class="nodata">
        {{text}}
    </div>
</template>

<script>

export default {
    name: 'tr-no-data',
    props: {
        text: {
            type: String,
            default: '暂无数据'
        }
    },
    data() {
        return {
        }
    },
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.nodata{
    position: absolute;
    top: 0;
    background: $bg_card;
    width: 100%;
    height: 100%;
    font-size: 12px;
    align-items:center; 
    justify-content:center;
    display: flex;
    display: -webkit-flex;
    color: $input_bg;
}
</style>