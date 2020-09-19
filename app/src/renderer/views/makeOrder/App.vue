<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>

export default {
    name: 'app',

    mounted(){
        this.removeLoadingMask();
        this.$store.dispatch('getTdMdList');
        this.$store.dispatch('getStrategyList');
        this.$store.dispatch('getAccountSourceConfig');
        this.$store.dispatch('getKungfuConfig');
    },

    methods: {
        removeLoadingMask () {
            //remove loading mask
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/base.scss';
 #app{
   height: 100%;
   background: $login-bg;
 }

 .system-prepare-dialog {
    .tr-dot-content {
        margin-right: 5px;
    }
 }

</style>