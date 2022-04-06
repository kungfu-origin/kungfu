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
                    <a-select
                        default-value="Spaces"
                        :bordered="false"
                        :dropdownMatchSelectWidth=false
                        :showArrow="false"
                        placement="topLeft"
                        @change="handleChange"
                    >
                        <a-select-option
                            v-for="item in options"
                            :key="item.key"
                        >{{item.key}}</a-select-option>
                    </a-select>
                </div>
            </a-layout>
        </a-layout>
    </div>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia';
import { ref } from 'vue';
import { useCodeStore } from '../store/codeStore';

const store = useCodeStore()
const { globallSetting } = storeToRefs(store)
const tabSpaceType = globallSetting.value?.code?.tabSpaceType || 'Spaces';
const type = tabSpaceType
const options = ref([{
    name: 'Indent Using Spaces',
    tip: type === 'Spaces' ? 'change view' : '',
    key: 'Spaces',
    active: type === 'Spaces',
},
{
    name: 'Indent Using Tabs',
    bottomLine: true,
    tip: type === 'Tabs' ? 'change view' : '',
    key: 'Tabs',
    active: type === 'Tabs',
},])


function handleChange (val) {
    console.log(val);
    
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
        .space-tab {
            border: none;
        }
    }
}
</style>
