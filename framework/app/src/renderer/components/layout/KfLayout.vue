<script setup lang="ts">
import { SlidersOutlined, SettingOutlined } from '@ant-design/icons-vue';
import { useExtConfigsRelated } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfProcessStatusController from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfProcessStatusController.vue';
import { computed, getCurrentInstance, onUnmounted, ref } from 'vue';
import globalBus from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/globalBus';
import KfGlobalSettingModal from '../public/KfGlobalSettingModal.vue';
const logo = require('@kungfu-trader/kungfu-app/src/renderer/assets/svg/LOGO.svg');

interface LayoutProps {}
defineProps<LayoutProps>();

const app = getCurrentInstance();
const globalSettingModalVisible = ref<boolean>(false);
const menuSelectedKeys = ref<string[]>(['main']);

const busSubscription = globalBus.subscribe((data: KfBusEvent) => {
  if (data.tag === 'main') {
    switch (data.name) {
      case 'open-setting-dialog':
        globalSettingModalVisible.value = true;
    }
  }
});

const { uiExtConfigs } = useExtConfigsRelated();
const sidebarFooterComponentKeys = computed(() => {
  return Object.keys(uiExtConfigs.data).filter(
    (key) => uiExtConfigs.data[key].position === 'sidebar_footer',
  );
});

const sidebarComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.data)
    .filter((key) => uiExtConfigs.data[key].position === 'sidebar')
    .map((key) => {
      return {
        ...uiExtConfigs.data[key],
        key,
      };
    });
});

onUnmounted(() => {
  busSubscription.unsubscribe();
});

function handleToPage(pathname: string) {
  if (app.proxy) {
    app.proxy.$router.push(pathname);
  }
}
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
          v-model:selectedKeys="menuSelectedKeys"
        >
          <a-menu-item key="main" @click="handleToPage('/')">
            <template #icon>
              <sliders-outlined style="font-size: 24px" />
            </template>
            <span>主面板</span>
          </a-menu-item>
          <a-menu-item
            v-for="config in sidebarComponentConfigs"
            :key="config.key"
            @click="handleToPage(`/${config.key}`)"
          >
            <template #icon>
              <component :is="config.key"></component>
            </template>
            <span>{{ config.name }}</span>
          </a-menu-item>
        </a-menu>
        <div class="kf-sidebar-footer__warp">
          <div
            class="kf-sidebar-footer-btn__warp"
            v-for="key in sidebarFooterComponentKeys"
          >
            <component :is="key"></component>
          </div>
          <div
            class="kf-sidebar-footer-btn__warp"
            @click="globalSettingModalVisible = true"
          >
            <setting-outlined class="kf-hover" style="font-size: 24px" />
          </div>
        </div>
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
    <KfGlobalSettingModal
      v-if="globalSettingModalVisible"
      v-model:visible="globalSettingModalVisible"
    ></KfGlobalSettingModal>
  </a-layout>
</template>

<style lang="less">
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/variables.less';

.ant-layout {
  height: 100%;

  .ant-layout-sider {
    .ant-layout-sider-children {
      display: flex;
      flex-direction: column;
      justify-content: space-between;
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

      .kf-sidebar-footer__warp {
        .kf-sidebar-footer-btn__warp {
          width: 100%;
          height: 56px;
          line-height: 56px;
          text-align: center;

          .anticon {
            font-size: 24px;
            &:hover {
              color: @primary-color;
              cursor: pointer;
            }
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
