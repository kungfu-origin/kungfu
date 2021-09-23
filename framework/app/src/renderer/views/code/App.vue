<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>

import { ipcEmitDataByName } from '@/ipcMsg/emitter';

export default {
    name: 'app',

    mounted(){
        this.removeLoadingMask();

        ipcEmitDataByName('strategyList')
            .then(({ data }) => {
                this.$store.dispatch('setStrategyList', data)
            })

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


</style>