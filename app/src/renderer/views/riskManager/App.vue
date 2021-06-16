<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>

import { ipcEmitDataByName } from '@/ipcMsg/emitter';
import { setTimerPromiseTask } from '__gUtils/busiUtils';

export default {
    name: 'app',

    mounted(){
        this.removeLoadingMask();

        ipcEmitDataByName('tdMdList')
            .then(({ data }) => {
                const { mdList, tdList } = data;
                this.$store.dispatch('setTdList', tdList)
                this.$store.dispatch('setMdList', mdList)
            })

        ipcEmitDataByName('strategyList')
            .then(({ data }) => {
                this.$store.dispatch('setStrategyList', data)

            })

        setTimerPromiseTask(() => ipcEmitDataByName('accountsAsset')
            .then(({ data }) => {
                this.$store.dispatch('setAccountsAsset', data);
            }),
            3000
        )

        this.$store.dispatch('getAccountSourceConfig');
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