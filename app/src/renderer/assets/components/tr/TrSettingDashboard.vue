<template>
    <div>
        <div class="tr-setting-warp" @click="handleClose"></div>
        <div class="tr-setting-dashboard">
            <ul class="tr-setting-dashboard-content">
                <li v-for="item in infoList" :key="item.key" :class="{
                    'setting-item': true, 
                    'bottom-line': item.bottomLine === true,
                    'active': item.active
                }"
                @click="handleConfrimSelect(item)"
                >
                    <span class="name fl">{{item.name}}</span>
                    <span class="tip fr">{{item.tip || ''}}</span>
                </li>
            </ul>
        </div>
    </div>
</template>
<script>
export default {
    name: 'tr-setting-dashboard',
    props: {

        infoList: {
            type: Array,
            default: []
        }
    },

    methods: {
        handleConfrimSelect(item) {
            const t = this;
            t.$emit('selected', item)
            t.$nextTick()
            .then(() => t.handleClose())
        },

        handleClose() {
            const t = this;
            t.$emit('update:visible', false)
        }
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.tr-setting-warp{
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: 998
}
.tr-setting-dashboard {
    position: fixed;
    width: 500px;
    top: 0px;
    left: 50%;
    transform: translateX(-50%);
    height: auto;
    z-index: 999;
    background: $bg_card;
    border-radius: 0 0 3px 3px;
    box-shadow: 0px 0px 30px $bg;
    .setting-item {
        width: 100%;
        height: 40px;
        line-height: 30px;
        font-size: 14px;
        padding: 5px 10px;
        color: $font;
        box-sizing: border-box;
        .tip {
            font-size: 14px;
            color: $icon
        }
    }
    .setting-item.active {
        background: $bg_light2;
    }
    .setting-item:hover {
        color: $font_5
    }
    .setting-item:active {
        background: $bg_light;
    }
    
}
</style>