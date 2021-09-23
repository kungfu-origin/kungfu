<template>
    <el-container class="tr-container">
        <!-- 头部 -->
        <el-container>
            <el-main class="tr-main">
                <slot></slot>
            </el-main>
        </el-container>

        <!-- 底部 -->
        <el-footer :height="!ifFooter ? '0' : '30px'">
            <div class="footer-content" v-if="ifFooter">
                <tr-footer-item class="fr">
                    <CodeSpaceTab></CodeSpaceTab>
                </tr-footer-item>
            </div>
        </el-footer>
    </el-container>
</template>

<script>

import CoreStatus from './components/CoreStatus';
import CodeSpaceTab from './components/CodeSpaceTab';


export default {
    name: 'main-content',
    props: {
        ifSideBar: {
            type: Boolean,
            default: true
        },

        ifFooter: {
            type: Boolean,
            default: true
        }
    },

    data() {
        this.LOGO = require('../../assets/img/LOGO.svg');
        return {
            infoListVisiblity: false,
        }
    },

    components: {
		CoreStatus,
        CodeSpaceTab,
    },

    methods:{
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
@import '@/assets/scss/skin.scss';

.kf-footer-popover{
    box-shadow: 0px 0px 30px $bg
}

.container-logo{
        height: 64px;
        width: 64px;
        line-height: 64px;
        text-align: center;
        float: left;
        background: $vi;
        i{
            font-size: 50px;
            color: white;
        }
}
</style>

