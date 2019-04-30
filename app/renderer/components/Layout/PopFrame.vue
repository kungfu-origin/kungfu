<template>
    <main-content :ifSideBar="ifSideBar">
        <el-main class="pop-main">
            <div :class="{'top': true, 'no-bg': noBg}">
                <div class="main-breadcrumb">
                    <slot name="breadcrumb"></slot>
                </div>
                <div class="top-right">
                    <slot name="operation"></slot>
                </div>
            </div>
            
            <div class="pop-content">
                <slot name="content"></slot>
            </div>
        </el-main>
    </main-content>

</template>

<script>

import MainHeader from './components/MainHeader'

export default {
    name: 'pop-frame',
    props: {
        ifSideBar: {
            type: Boolean,
            default: true
        },
        noBg: {
            type: Boolean,
            default: false
        }
    },
    data() {
        return {
        }
    },
    components: {
        MainHeader,
    },

    methods:{
        
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
</style>

