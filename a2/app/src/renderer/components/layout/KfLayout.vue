<script setup lang="ts">
import { SlidersOutlined } from '@ant-design/icons-vue';
import KfProcessStatusController from '@renderer/components/layout/KfProcessStatusController.vue';
const logo = require('@renderer/assets/svg/LOGO.svg');

interface LayoutProps {}
defineProps<LayoutProps>();
</script>
<template>
    <a-layout>
        <a-layout>
            <a-layout-sider width="64px">
                <div class="kf-header-logo">
                    <img :src="logo" />
                </div>
                <a-menu
                    mode="vertical"
                    style="width: 64px"
                    :selectedKeys="['main']"
                >
                    <a-menu-item key="main">
                        <template #icon>
                            <sliders-outlined style="font-size: 18px" />
                        </template>
                        <span>主面板</span>
                    </a-menu-item>
                </a-menu>
            </a-layout-sider>
            <a-layout style="padding: 0px 8px 0 8px; box-sizing: border-box">
                <a-layout-content>
                    <slot></slot>
                </a-layout-content>
            </a-layout>
        </a-layout>
        <a-layout-footer>
            <KfProcessStatusController></KfProcessStatusController>
        </a-layout-footer>
    </a-layout>
</template>

<style lang="less">
@import '@renderer/assets/less/variables.less';
.ant-layout {
    height: 100%;

    .ant-layout-sider {
        .ant-layout-sider-children {
            position: relative;

            .kf-header-logo {
                width: 100%;
                height: @layout-side-width;
                background: @primary-color;
                position: relative;

                img {
                    position: absolute;
                    width: 100%;
                    height: 100%;
                    left: 0;
                    top: 0;
                }
            }

            .ant-menu.ant-menu-root.ant-menu-vertical {
                position: absolute;
                top: 40%;
                transform: translateY(-50%);
                left: 0;

                .ant-menu-item {
                    height: @layout-side-width;
                    line-height: @layout-side-width;
                    display: flex;
                    justify-content: space-evenly;
                    flex-direction: column;

                    > span {
                        display: block;
                    }

                    .ant-menu-title-content {
                        height: auto;
                        line-height: 1;
                        margin-left: 0;
                    }
                }
            }
        }
    }

    .ant-layout-footer {
        height: @layout-footer-height;
        line-height: @layout-footer-height;
        padding: 0 8px 0 0;
    }
}
</style>
