<template>
    <el-container class="tr-container">
        <!-- 头部 -->
        <el-container>
            <!-- 左侧边栏 -->
            <el-aside :width="'64px'" class="tr-aside" v-if="ifSideBar">
                <div class="container-logo">
                    <img width="64" height="50" src="@/assets/img/LOGO.svg"/>
                </div>
                <tr-menu>
                    <tr-menu-item
                    route="/"
                    >
                        <i class="el-icon-s-platform" ></i>
                        <template slot="title">账户</template>
                    </tr-menu-item>
                     <tr-menu-item
                    route="/market-filter"
                    >
                        <i class="el-icon-s-operation"></i>
                        <template slot="title">选股</template>
                    </tr-menu-item>
                    <tr-menu-item
                    route="/market-filter"
                    >
                        <i class="el-icon-s-operation" ></i>
                        <template slot="title">行情</template>
                    </tr-menu-item>
                    <tr-menu-item
                    route="/strategy"
                    >
                        <i class="el-icon-s-order" ></i>
                        <template slot="title">策略</template>
                    </tr-menu-item>
                    <!-- <tr-menu-item
                    route="/data-management"
                    >
                        <i class="el-icon-s-data" ></i>
                        <template slot="title">数据</template>
                    </tr-menu-item> -->
                </tr-menu>
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
        return {
            infoListVisiblity: false,
        }
    },

    components: {
        EngineStatus,
		CoreStatus,
    },
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
@import '@/assets/scss/skin.scss';

.kf-footer-popover{
    box-shadow: 0px 0px 30px $bg
}

.container-logo{
        height: 50px;
        width: 64px;
        line-height: 50px;
        text-align: center;
        float: left;
        background: $vi;
        i{
            font-size: 50px;
            color: white;
        }
}
</style>

