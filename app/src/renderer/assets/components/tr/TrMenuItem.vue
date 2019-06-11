<template>
    <div 
    class="tr-menu-item" 
    @click="handleItemClick"
    :class="{
      'is-active': active
    }"
    > 
        <div class="item-icon">
            <slot></slot>
        </div>
        <div class="item-name">
            <slot name="title"></slot>
        </div>
    </div>
</template>

<script>
export default {
    name: 'tr-menu-item',
    props: {
        route: {
            type: String,
            required: true
        }
    },
    data() {
        return {

        }
    },

    inject: ['menu'], //与provide组合运用

    computed: {
        //当点击的是该项的时候,或者路由相等的时候
        active() {
            //考虑到跳转到二级目录的时候，导航也要选中，但要排除首页的情况
            return this.$route.path.indexOf(this.route) != -1 && this.route != '/' || this.route === this.$route.path;
            // return this.route === this.menu.activeRouter || this.$route.path.indexOf(this.route) != -1;
        }
    },

    methods: {
        handleItemClick() {
            this.dispatch('tr-menu','item-click', this.route);
        },

        dispatch(componentName, eventName, params) {
            var parent = this.$parent || this.$root;
            var name = parent.$options._componentTag;
            while (parent && (!name || name !== componentName)) {
                parent = parent.$parent;
        
                if (parent) {
                    name = parent.$options._componentTag;
                }
            }
            if (parent) {
            parent.$emit.apply(parent, [eventName].concat(params));
            }
        }
    }
}
</script>

<style lang="scss" scoped>
@import '@/assets/scss/skin.scss';
.tr-menu-item{
    padding: 12px 0;
    &:hover{
        background: $input_bg;
        cursor: pointer;
        
    }
    .item-icon{
        text-align: center;
        
        i{
            font-size: 20px;
        }
    }
    .item-name{
        font-size: 11px;
        text-align: center;
        padding-top:4px;
        color:$font;
    }

}
.is-active{
    background:  $input_bg;
    .item-icon{
        i{
        color: $vi;
        }
    }
        .item-name{
        color: $font_1;
    }
    
}


</style>