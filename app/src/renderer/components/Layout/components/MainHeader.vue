<template>
    <div class="main-header">
        <div>
            <div class="container-logo">
                <!-- <i class="iconfont tr-LOGO-1"></i> -->
                <img width="64" height="50" src="../../../assets/img/LOGO.svg"/>
            </div>
            <div class="logo-name">
                <!-- KungFu -->
                <!-- <img src="../../../assets/img/logo.png"/> -->
                <i class="iconfont tr-logo1"></i>
            </div>
        </div>
     
    </div>
</template>
<script>

export default {
    name: 'mian-header',
    data() {
        return {
        }
    },

    methods:{
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.main-header{
    .container-logo{
        height: 50px;
        width: 64px;
        line-height: 50px;
        text-align: center;
        // background-size: 100%;
        float: left;
        background: $vi;
        i{
            font-size: 50px;
            color: white;
        }
    }
    .logo-name{
        float: left;
        i{
            font-size: 50px;
            color: white;
        }
    }
}
</style>

