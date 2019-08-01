<template>
    <el-container class="tr-container">
        <!-- 头部 -->
        <el-header height="50px">
            <MainHeader/>
        </el-header>

        <el-container>
            <!-- 左侧边栏 -->
            <el-aside :width="'64px'" class="tr-aside" v-if="ifSideBar">
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
                     <tr-menu-item
                    route="/extension"
                    >
                        <i class="el-icon-box" ></i>
                        <template slot="title">插件</template>
                    </tr-menu-item>
                </tr-menu>
            </el-aside>
            <el-main class="tr-main">
                <slot></slot>
            </el-main>
        </el-container>

        <!-- 底部 -->
        <el-footer height="30px">
            <div class="footer-content">
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
import {deepClone} from '__gUtils/busiUtils';
import {mapState, mapGetters} from 'vuex';
import EngineStatus from './components/EngineStatus';
import CoreStatus from './components/CoreStatus';
import MainHeader from './components/MainHeader';

export default {
    name: 'main-content',
    props: {
        ifSideBar: {
            type: Boolean,
            default: true
        }
    },
    data() {
        return {
            infoListVisiblity: false,
            navOpend: deepClone(this.$store.state.BASE.navOpend),
        }
    },

    components: {
        EngineStatus,
        MainHeader,
        CoreStatus
    },

    computed:{
        ...mapState({
            tradeLogVisible: state => state.BASE.tradeLogVisible
        })
    },

    methods:{
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
@import '@/assets/scss/skin.scss';
</style>

