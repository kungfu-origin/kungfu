<template>
    <div class="app__warp">
        <router-view />
    </div>
</template>

<script lang="ts">
import { defineComponent } from '@vue/runtime-core';
export default defineComponent({
    mounted() {
        this.removeLoadingMask();
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
