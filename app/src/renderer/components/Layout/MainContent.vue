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
            <div class="footer-content" >
                <tr-footer-item class="fr" v-if="currentRouter !== 'code'">
                    <CoreStatus></CoreStatus>
                </tr-footer-item>
             
                <tr-footer-item class="fr" v-if="currentRouter !== 'code'">
                    <EngineStatus></EngineStatus>
                </tr-footer-item>

				<tr-footer-item class="fr" v-if="currentRouter === 'code'">
					<CodeSpaceTab></CodeSpaceTab>
				</tr-footer-item>
            </div>
        </el-footer>

    </el-container>
</template>

<script>
import { deepClone } from '__gUtils/busiUtils';
import { mapState, mapGetters } from 'vuex';
import EngineStatus from './components/EngineStatus';
import CoreStatus from './components/CoreStatus';
import CodeSpaceTab from './components/CodeSpaceTab';

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
            currentRouter: '',
            infoListVisiblity: false,
            navOpend: deepClone(this.$store.state.BASE.navOpend),
        }
    },

    created() {
        this.currentRouter = this.$router.history.current.name
    },

    components: {
        EngineStatus,
		CoreStatus,
		CodeSpaceTab
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

