<template>
    <div class="tr-menu">
        <slot></slot>
    </div>
</template>

<script>

export default {
    name: 'tr-menu',
    props: {
        router: {
            type: Boolean,
            default: true
        },
        defaultActive: {
            type: String,
            default: ''
        },
    },
    data() {
        return {
            activeRouter: this.defaultActive
        }
    },
    provide() {
        return {
            menu: this
        }
    },

    mounted(){
        this.$on('item-click', this.handleItemClick);
    },

    methods: {
        handleItemClick(item) {
            const t = this
            t.activeRouter = item
            if (t.router) {
                t.$router.push(item)
            }
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.tr-menu{
    position:absolute;
    width: 100%;
    top: calc(50% - 70px);
    left: 50%;
    // height: 50%;
    transform: translateX(-50%) translateY(-50%);
}
</style>