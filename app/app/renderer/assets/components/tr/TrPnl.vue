<template>
    <span class="tr-pnl" 
    :class="{'color-red':$utils.toDecimal(value) > 0, 'color-green':$utils.toDecimal(value) < 0}">
        {{$utils.toDecimal(value)}}
    </span>
    
</template>
<script>
export default {
    name: "tr-pnl",
    props: {
        value: ''
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
</style>
