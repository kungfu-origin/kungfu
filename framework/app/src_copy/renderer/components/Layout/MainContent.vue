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
                    <div class="setting-item head-image" id="user-info-header" v-else @click="handleOpenMenu">
                        <span slot="reference" class="kf-user-info-popover-entry">
                            <img :src="photo" width="100%" height="100%" alt="">
                        </span>
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

import { remote } from 'electron'
import EngineStatus from '@components/Layout/components/EngineStatus';
import CoreStatus from '@components/Layout/components/CoreStatus';
import authMixin from '@components/Base/js/authMixin';

const { Menu, getCurrentWindow } = remote;


export default {
    name: 'main-content',

    mixins: [ authMixin ],

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
        this.LOGO = require('@/assets/img/LOGO.svg');
        return {
            infoListVisiblity: false,
        }
    },

    components: {
        EngineStatus,
		CoreStatus,
    },

    methods: {
        handleOpenMenu () {
            const template = [{
		        label: `${this.showName}`,
                submenu: [
			        { label: "个人中心", click: () => this.openLoginInfoDialog() },
			        { label: "退出登录", click: () => this.authLogout() }
                ]
            }]
            const menu = Menu.buildFromTemplate(template);
            this.getHeaderImageLocation()
                .then(({ x, y }) => {
                    return menu.popup({
                        window: getCurrentWindow(),
                        x,
                        y
                    })
                })

        },

        handleOpenGlobalSetting () {
            this.$bus.$emit('open-global-setting');
        },

        handleLogin () {
            this.$bus.$emit('open-authing-guard')
        },

        getHeaderImageLocation () {
            const $headerImage = document.querySelector('#user-info-header');
            const offsetY = $headerImage.offsetTop;
            const offsetX = $headerImage.offsetLeft;

            return Promise.resolve({
                x: offsetX + 64, //64 is header image width
                y: offsetY
            })
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

            &.head-image {
                text-align: center;

                img {
                    display: inline-block;
                    width: 40px;
                    height: 40px;
                    line-height: 40px;
                    border-radius: 50%;
                    overflow: hidden;
                }
            }
        }
    }
    
}




</style>

