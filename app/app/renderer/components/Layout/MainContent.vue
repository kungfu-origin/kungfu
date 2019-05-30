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
                    <EngineStatus></EngineStatus>
                </tr-footer-item>
            </div>
        </el-footer>

    </el-container>
</template>

<script>
import {deepClone} from '@/assets/js/utils';
import {mapState, mapGetters} from 'vuex';
import EngineStatus from './components/EngineStatus';
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
        MainHeader
    },

    computed:{
        ...mapState({
            tradeLogVisible: state => state.BASE.tradeLogVisible
        }),
        ...mapGetters({
        }),
        
        //当没有柜台的时候，不显示
        showEngines() {
        }
    },

    methods:{
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/layout.scss';
@import '@/assets/scss/skin.scss';
// .title-bar{
//     height: 24px;
//     line-height: 24px;
//     background: $bg;
//     border-bottom: 1px solid $bg_dark;
// }
</style>

