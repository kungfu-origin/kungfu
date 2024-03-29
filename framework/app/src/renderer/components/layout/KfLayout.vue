<script setup lang="ts">
import { SlidersOutlined, SettingOutlined } from '@ant-design/icons-vue';
import KfProcessStatusController from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfProcessStatusController.vue';
import KfUpdateController from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfUpdateController.vue';
import { computed, getCurrentInstance, onBeforeUnmount, ref } from 'vue';
import { useExtConfigsRelated } from '../../assets/methods/actionsUtils';
import { isUpdateVersionLogicEnable } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import KfGlobalSettingModal from '../public/KfGlobalSettingModal.vue';
import { useLanguage } from '@kungfu-trader/kungfu-js-api/language';
import {
  getLogoPath,
  isDefaultLogo,
} from '@kungfu-trader/kungfu-js-api/config/brand';

const logoPath = isDefaultLogo()
  ? require('@kungfu-trader/kungfu-app/src/renderer/assets/svg/LOGO.svg')
  : getLogoPath();

const app = getCurrentInstance();
const globalSettingModalVisible = ref<boolean>(false);
const menuSelectedKeys = ref<string[]>(['main']);

const { uiExtConfigs } = useExtConfigsRelated();
const { isLanguageKeyAvailable } = useLanguage();

const sidebarFooterComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.value)
    .filter((key) => uiExtConfigs.value[key].position === 'sidebar_footer')
    .map((key) => {
      return {
        ...uiExtConfigs.value[key],
        key,
      };
    });
});

const footerComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.value)
    .filter((key) => uiExtConfigs.value[key].position === 'footer')
    .map((key) => {
      return {
        ...uiExtConfigs.value[key],
        key,
      };
    });
});

const sidebarComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.value)
    .filter((key) => uiExtConfigs.value[key].position === 'sidebar')
    .map((key) => {
      return {
        ...uiExtConfigs.value[key],
        key,
      };
    });
});

const busSubscription = globalBus.subscribe((data: KfEvent.KfBusEvent) => {
  if (data.tag === 'main') {
    switch (data.name) {
      case 'open-setting-dialog':
        globalSettingModalVisible.value = true;
    }
  }
});

onBeforeUnmount(() => {
  busSubscription.unsubscribe();
});

function handleToPage(pathname: string) {
  if (app?.proxy) {
    app.proxy.$router.push(pathname);
  }
}
</script>
<template>
  <a-layout>
    <a-layout>
      <a-layout-sider width="64px">
        <div class="kf-header-logo">
          <img :src="logoPath" />
        </div>
        <a-menu
          v-model:selectedKeys="menuSelectedKeys"
          mode="vertical"
          style="width: 64px"
        >
          <a-menu-item key="main" @click="handleToPage('/')">
            <template #icon>
              <sliders-outlined style="font-size: 24px" />
            </template>
            <span>{{ $t('baseConfig.main_panel') }}</span>
          </a-menu-item>
          <a-menu-item
            v-for="config in sidebarComponentConfigs"
            :key="config.key"
            @click="handleToPage(`/${config.key}`)"
          >
            <template #icon>
              <component :is="config.key"></component>
            </template>
            <span>
              {{
                isLanguageKeyAvailable(config.name)
                  ? $t(config.name)
                  : config.name
              }}
            </span>
          </a-menu-item>
        </a-menu>
        <div class="kf-sidebar-footer__warp">
          <div
            v-for="config in sidebarFooterComponentConfigs"
            :key="config.key"
            class="kf-sidebar-footer-btn__warp"
            :title="
              isLanguageKeyAvailable(config.name)
                ? $t(config.name)
                : config.name
            "
          >
            <component :is="config.key"></component>
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
      <KfProcessStatusController
        class="kf-footer-box__warp"
      ></KfProcessStatusController>
      <KfUpdateController
        v-if="isUpdateVersionLogicEnable()"
        class="kf-footer-box__warp"
      ></KfUpdateController>
      <div
        v-for="config in footerComponentConfigs"
        :key="config.key"
        class="kf-footer-box__warp"
      >
        <component :is="config.key"></component>
      </div>
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
          align-items: center;
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

    .kf-footer-box__warp {
      float: right;
      height: 100%;
      padding: 0 8px;
      cursor: pointer;
      font-size: 12px;
      font-weight: bold;
      font-size: 12px;
      color: @primary-color;

      &:hover {
        background: @item-active-bg;
      }
    }
  }
}
</style>
