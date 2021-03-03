<template>
    <span :class="[ ...classNameResolved, 'blink-num', colorClassName ]" :key="`${theKey}_${num}`">{{ num }}</span>
</template>

<script>
export default {

    props: {
        num: {
            type: [ String, Number ],
            default: '--'
        },

        theKey: {
            type: [String, Number],
            required: true,
            default: ''
        },

        className: {
            type: [ Object ],
            default: ''
        }
    },

    data () {
        return {
            colorClassName: ''
        }
    },

    computed: {
        classNameResolved () {
            return Object.keys(this.className || {}).filter(key => !!this.className[key])
        }  
    },

    watch: {
        num (newVal, oldVal) {
            if (Number.isNaN(+oldVal)) return ''

            if (+newVal > +oldVal) {
                this.colorClassName = 'red-blink'
            } else if (+newVal < +oldVal) {
                this.colorClassName = 'green-blink'
            } else {
                return ''
            }
        }
    }
}
</script>

<style lang="scss">

@import '@/assets/scss/skin.scss';

.blink-num {
    
    &.red-blink {
        animation: nanoRedBlink 1.1s 1;
    }

    &.green-blink {
        animation: nanoGreenBlink 1.1s 1;
    }
}


</style>