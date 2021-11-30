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
                    v-model:selectedKeys="selectedKeys"
                >
                    <a-menu-item key="1">
                        <template #icon>
                            <TeamOutlined style="font-size: 16px" />
                        </template>
                        <span>账户</span>
                    </a-menu-item>
                    <a-menu-item key="2">
                        <template #icon>
                            <ThunderboltOutlined style="font-size: 16px" />
                        </template>
                        <span>策略</span>
                    </a-menu-item>
                </a-menu>
            </a-layout-sider>
            <a-layout style="padding: 0px 8px 0 8px; box-sizing: border-box">
                <a-layout-content>
                    <slot></slot>
                </a-layout-content>
            </a-layout>
        </a-layout>
        <a-layout-footer></a-layout-footer>
    </a-layout>
</template>

<script lang="ts">
import { defineComponent, reactive, toRefs } from 'vue';
import { TeamOutlined, ThunderboltOutlined } from '@ant-design/icons-vue';

export default defineComponent({
    components: {
        TeamOutlined,
        ThunderboltOutlined,
    },

    setup() {
        const state = reactive({
            selectedKeys: ['1'],
        });

        return {
            ...toRefs(state),
            logo: require('@renderer/assets/svg/LOGO.svg'),
        };
    },
});
</script>

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
        padding: 0;
    }
}
</style>
