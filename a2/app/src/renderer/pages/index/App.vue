<template>
    <div class="app__warp">
        <router-view />
        <test></test>
    </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    mounted() {
        this.removeLoadingMask();

        console.log(this.$root?.$options.components, '--');
    },

    methods: {
        removeLoadingMask() {
            //remove loading mask
            const $loadingMask = document.getElementById('loading');
            if ($loadingMask) $loadingMask.remove();
        },
    },
});
</script>

<style lang="less">
@import '@renderer/assets/less/base.less';
@import '@renderer/assets/less/coverAnt.less';

#app {
    width: 100%;
    height: 100%;
    font-family: Avenir, Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;

    .app__warp {
        height: 100%;
        width: 100%;
    }
}
</style>
