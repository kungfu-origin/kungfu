<template>
    <a-config-provider :locale="zhCN">
        <div class="app__warp">
            <router-view />
        </div>
    </a-config-provider>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import zhCN from 'ant-design-vue/es/locale/zh_CN';

export default defineComponent({
    setup() {
        return {
            zhCN,
        };
    },

    mounted() {
        this.removeLoadingMask();

        window.addEventListener('resize', () => {
            this.$bus.next({
                tag: 'resize',
            } as ResizeEvent);
        });
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
@import '@renderer/assets/less/public.less';
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
