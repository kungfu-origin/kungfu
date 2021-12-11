<template>
    <a-config-provider :locale="zhCN">
        <div class="app__warp">
            <router-view />
        </div>
    </a-config-provider>
</template>

<script lang="ts">
import { defineComponent, getCurrentInstance, onMounted } from 'vue';
import zhCN from 'ant-design-vue/es/locale/zh_CN';
import { useGlobalStore } from './store/global';
import { removeLoadingMask } from '@renderer/assets/methods/pureUiUtils';

export default defineComponent({
    setup() {
        const app = getCurrentInstance();

        useGlobalStore().setKfConfigList();
        useGlobalStore().setKfExtConfigs();

        onMounted(() => {
            removeLoadingMask();
            window.addEventListener('resize', () => {
                app?.proxy &&
                    app?.proxy.$bus.next({
                        tag: 'resize',
                    } as ResizeEvent);
            });
        });

        return {
            zhCN,
        };
    },

    mounted() {
        this.$useGlobalStore().setKfExtConfigs;
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

    .app__warp {
        height: 100%;
        width: 100%;
    }
}
</style>
