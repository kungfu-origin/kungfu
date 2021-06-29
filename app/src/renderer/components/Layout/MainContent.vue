<template>
    <el-container class="tr-container">
        <!-- 头部 -->
        <el-container>
            <!-- 左侧边栏 -->
            <el-aside :width="'64px'" class="tr-aside" v-if="ifSideBar">
                <div class="container-logo">
                    <img width="64" height="64" :src="LOGO"/>
                </div>
                <tr-menu>
                    <tr-menu-item
                    route="/"
                    >
                        <i class="el-icon-s-platform" ></i>
                        <template slot="title">账户</template>
                    </tr-menu-item>
                    <tr-menu-item
                    route="/strategy"
                    >
                        <i class="el-icon-s-order" ></i>
                        <template slot="title">策略</template>
                    </tr-menu-item>
                </tr-menu>
                <div class="tr-aside-bottom-settings">
                    <div class="setting-item" v-if="!isActivated" @click="handleLogin">
                        <i class="el-icon-user" ></i>
                    </div>
                    <div class="setting-item" v-else>
                        <img :src="photo" width="100%" height="100%" alt="">
                    </div>
                    <div class="setting-item" @click="handleOpenGlobalSetting">
                        <i class="el-icon-setting"></i>
                    </div>
                </div>
            </el-aside>
            <el-main class="tr-main">
                <slot></slot>
            </el-main>
        </el-container>

        <!-- 底部 -->
        <el-footer :height="!ifFooter ? '0' : '30px'">
            <div class="footer-content" v-if="ifFooter">
                <tr-footer-item class="fr">
                    <CoreStatus></CoreStatus>
                </tr-footer-item>
            
                <tr-footer-item class="fr">
                    <EngineStatus></EngineStatus>
                </tr-footer-item>
            </div>
        </el-footer>
    </el-container>
</template>

<script>

import { mapState } from 'vuex';
import EngineStatus from './components/EngineStatus';
import CoreStatus from './components/CoreStatus';

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

    computed: {
        ...mapState({
            loginInfo: state => state.BASE.loginInfo || {}
        }),

        isActivated () {
            return this.loginInfo.status || false;
        },

        photo () {
            return this.loginInfo.photo || false
        }
    },

    components: {
        EngineStatus,
		CoreStatus,
    },

    methods: {
        handleOpenGlobalSetting () {
            this.$bus.$emit('open-global-setting');
        },

        handleLogin () {
            this.$bus.$emit('open-authing-guard')
        },
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
@import '@/assets/scss/skin.scss';

.kf-footer-popover{
    box-shadow: 0px 0px 30px $bg
}

.el-aside.tr-aside {
    display: flex;
    flex-direction: column;

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


    .tr-aside-bottom-settings {

        .setting-item {
            width: 100%;
            height: 50px;
            line-height: 50px;
            text-align: center;
            font-size: 24px;
            cursor: pointer;
        }
    }
    
}




</style>

