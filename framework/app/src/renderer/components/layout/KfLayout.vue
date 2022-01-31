<script setup lang="ts">
import { SlidersOutlined, SettingOutlined } from '@ant-design/icons-vue';
import bus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import KfProcessStatusController from '@renderer/components/layout/KfProcessStatusController.vue';
import { onUnmounted, ref } from 'vue';
import KfGlobalSettingModal from '../public/KfGlobalSettingModal.vue';
const logo = require('@renderer/assets/svg/LOGO.svg');

interface LayoutProps {}
defineProps<LayoutProps>();

const globalSettingModalVisible = ref<boolean>(false);

const busSubscription = bus.subscribe((data: KfBusEvent) => {
  if (data.tag === 'main') {
    switch (data.name) {
      case 'open-setting-dialog':
        globalSettingModalVisible.value = true;
    }
  }
});

onUnmounted(() => {
  busSubscription.unsubscribe();
});
</script>
<template>
  <a-layout>
    <a-layout>
      <a-layout-sider width="64px">
        <div class="kf-header-logo">
          <img :src="logo" />
        </div>
        <a-menu mode="vertical" style="width: 64px" :selectedKeys="['main']">
          <a-menu-item key="main">
            <template #icon>
              <sliders-outlined style="font-size: 24px" />
            </template>
            <span>主面板</span>
          </a-menu-item>
        </a-menu>
        <div
          class="kf-global-setting-btn"
          @click="globalSettingModalVisible = true"
        >
          <setting-outlined class="kf-hover" style="font-size: 24px" />
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
@import '@renderer/assets/less/variables.less';

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

      .kf-global-setting-btn {
        width: 100%;
        height: 64px;
        line-height: 64px;
        text-align: center;
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
