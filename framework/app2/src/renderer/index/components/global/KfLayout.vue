<template>
    <a-layout>
        <a-layout-sider width="60px"></a-layout-sider>
        <a-layout>
            <a-layout-content>
                <slot></slot>
            </a-layout-content>
        </a-layout>
    </a-layout>
</template>

<script lang="ts"></script>

<style lang="less">
.ant-layout {
    height: 100%;
}
</style>
