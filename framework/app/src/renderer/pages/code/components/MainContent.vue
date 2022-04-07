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
                            {{defaultValue}} <a-icon type="down" />
                        </a-button>
                        <template #overlay>
                            <a-menu>
                                <a-sub-menu
                                    v-for="item in options"
                                    :key="item.key"
                                    :title="item.name"
                                >
                                    <a-menu-item v-for="size in sizeOptions" @click="handleClick(item, size)">{{size.name}}</a-menu-item>
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
import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { storeToRefs } from 'pinia';
import { ref, toRaw, watch } from 'vue';
import { useCodeStore } from '../store/codeStore';

interface indent {
    name: string | number;
    key: string | number;
}
const store = useCodeStore()
const { globallSetting } = storeToRefs(store)

const defaultValue = ref<string>('')
watch(globallSetting.value, newSetting => {
    defaultValue.value = `${newSetting?.code?.tabSpaceType}: ${newSetting?.code?.tabSpaceSize}`
})

const options = ref<Array<indent>>([{
        name: 'Indent Using Spaces',
        key: 'Spaces',
    },
    {
        name: 'Indent Using Tabs',
        key: 'Tabs',
    }])
const sizeOptions = ref<Array<indent>>([{
        name: 2,
        key: 2,
    },
    {
        name: 4,
        key: 4,
    }])

function handleClick (type: indent, size: indent) {

    const setting: Record<string, Record<string, KungfuApi.KfConfigValue>> = deepClone(toRaw(globallSetting.value))
    setting.code.tabSpaceType = type.key
    setting.code.tabSpaceSize = size.key
    store.setGlobalSetting(setting)
    
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
