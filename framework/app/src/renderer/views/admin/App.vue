<template>
    <div id="app">
        <el-container>
            <el-header>
                <img width="64" height="100%" :src="LOGO"/>
                <div class="title">功夫管理员系统</div>
            </el-header>
            <el-main>
                <router-view></router-view>
            </el-main>
        </el-container>
    </div>
</template>
<script>

export default {
    name: 'app',

    data () {
        this.LOGO = require('../../assets/img/LOGO.svg');
        return {}
    },

    mounted () {
        this.removeLoadingMask();
        
        this.$store.dispatch('getTdMdList');
        this.$store.dispatch('getStrategyList');
        this.$store.dispatch('getAccountSourceConfig');
        this.$store.dispatch('getKungfuConfig');
        this.$store.dispatch('getExtensionConfigs')
    },

    methods: {
        removeLoadingMask () {
            //remove loading mask
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/base.scss';
 #app{
   height: 100%;
   background: $bg;

   .el-container {
       height: 100%;

       .el-header {
            padding: 0;
            height: 60px;
            line-height: 60px;
            background: $bg_card;

            img {
                background: $vi;
                float: left;
            }

           .title {
                display: inline-block;
                color: $icon;
                font-size: 24px;
                font-weight: 600;
                height: 60px;
                line-height: 60px;
                float: left;
                padding-left: 20px;
           }
       }

       .el-main {
           padding: 0;
       }
   }
 }

</style>