<template>
  <div class="coder-content">
    <a-layout>
      <a-layout>
        <a-layout-content>
          <slot></slot>
        </a-layout-content>
      </a-layout>
      <a-layout>
        <div class="footer">
          <a-dropdown :trigger="['click']">
            <a-button class="ant-dropdown-link">
              {{ defaultValue }}
            </a-button>
            <template #overlay>
              <a-menu>
                <a-sub-menu
                  v-for="item in options"
                  :key="item.key"
                  :title="item.name"
                >
                  <a-menu-item
                    v-for="size in sizeOptions"
                    @click="handleClick(item, size)"
                  >
                    {{ size.name }}
                  </a-menu-item>
                </a-sub-menu>
              </a-menu>
            </template>
          </a-dropdown>
        </div>
      </a-layout>
    </a-layout>
  </div>
</template>

<script setup lang="ts">
import {
  CodeTabSetting,
  CodeSizeSetting,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  SpaceTabSettingEnum,
  SpaceSizeSettingEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { storeToRefs } from 'pinia';
import { ref, toRaw, watch } from 'vue';
import { useCodeStore } from '../store/codeStore';

interface indent {
  name: string;
  key: string;
}
const store = useCodeStore();
const { globalSetting } = storeToRefs(store);
const indentUsingSpace: string =
  CodeTabSetting[SpaceTabSettingEnum.SPACES].name;
const indentUsingTab: string = CodeTabSetting[SpaceTabSettingEnum.TABS].name;
const sizeUsingTwo: string =
  CodeSizeSetting[SpaceSizeSettingEnum.TWOINDENT].name;
const sizeUsingFour: string =
  CodeSizeSetting[SpaceSizeSettingEnum.FOURINDENT].name;
const defaultValue = ref<string>(`${indentUsingSpace}: ${sizeUsingTwo}`);
watch(globalSetting.value, (newSetting) => {
  defaultValue.value = `${
    newSetting?.code?.tabSpaceType
      ? CodeTabSetting[newSetting?.code?.tabSpaceType]?.name
      : indentUsingSpace
  }: ${
    newSetting?.code?.tabSpaceSize
      ? CodeSizeSetting[newSetting?.code?.tabSpaceSize]?.name
      : sizeUsingFour
  }`;
});

const options = ref<Array<indent>>([
  {
    name: indentUsingSpace,
    key: indentUsingSpace,
  },
  {
    name: indentUsingTab,
    key: indentUsingTab,
  },
]);
const sizeOptions = ref<Array<indent>>([
  {
    name: sizeUsingTwo,
    key: sizeUsingTwo,
  },
  {
    name: sizeUsingFour,
    key: sizeUsingFour,
  },
]);

function handleSpaceType(type: string): string {
  let handledType = '';
  switch (type) {
    case indentUsingSpace:
      handledType = SpaceTabSettingEnum.SPACES;
      break;
    case indentUsingTab:
      handledType = SpaceTabSettingEnum.TABS;
      break;
  }
  return handledType;
}

function handleSpaceSize(type: string): string {
  let handledSize = '';
  switch (type) {
    case sizeUsingTwo:
      handledSize = SpaceSizeSettingEnum.TWOINDENT;
      break;
    case sizeUsingFour:
      handledSize = SpaceSizeSettingEnum.FOURINDENT;
      break;
  }
  return handledSize;
}

function handleClick(type: indent, size: indent) {
  let setting: Record<string, Record<string, KungfuApi.KfConfigValue>>;
  if (globalSetting.value != {} && globalSetting.value.code) {
    setting = deepClone(toRaw(globalSetting.value));
  } else {
    setting = {
      code: {
        tabSpaceType: '',
        tabSpaceSize: '',
      },
    };
  }
  setting.code.tabSpaceType =
    handleSpaceType(type.key) || SpaceTabSettingEnum.SPACES;
  setting.code.tabSpaceSize =
    handleSpaceSize(size.key) || SpaceSizeSettingEnum.TWOINDENT;
  store.setGlobalSetting(setting);
}
</script>

<style lang="less">
.coder-content {
  width: 100%;
  height: 100%;
  .footer {
    width: 100%;
    height: 100%;
    display: flex;
    align-items: center;
    justify-content: flex-end;
    .ant-dropdown-link {
      border: none;
    }
  }
}
</style>
