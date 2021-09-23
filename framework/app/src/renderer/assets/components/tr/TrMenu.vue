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
                t.$router.push(item).catch(err => {})
            }
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.tr-menu{
    display: flex;
    flex-direction: column;
    flex: 1;
    width: 100%;
    justify-content: center;
}
</style>