<template>
    <span :class="[ ...classNameResolved, 'blink-num', colorClassName, colorClass ]" :key="`${theKey}_${num}`">{{ num }}</span>
</template>

<script>
export default {
    name: 'tr-blink-num',

    props: {
        num: {
            type: [ String, Number ],
            default: '--'
        },

        theKey: {
            type: [ String, Number ],
            required: true,
            default: ''
        },

        className: {
            type: Object,
            default: () => ({})
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
        },

        colorClass () {
            if (Number.isNaN(+parseFloat(this.num))) return '';
            if (+parseFloat(this.num) < 0) return 'green-color'
        }
    },

    watch: {
        num (newVal, oldVal) {
            if (Number.isNaN(+parseFloat(oldVal))) return;

            let bgClass = '';

            if (+parseFloat(newVal) > +parseFloat(oldVal)) {
                bgClass = 'red-blink'
            } else if (+parseFloat(newVal) < +parseFloat(oldVal)) {
                bgClass = 'green-blink'
            } else {
                bgClass = ''
            }

            if (this.theKey.includes('remainDays')) {
                console.log(this.theKey)
                console.log(bgClass, newVal, oldVal, +parseFloat(newVal) < +parseFloat(oldVal), +parseFloat(newVal), +parseFloat(oldVal))
            }


            this.colorClassName = bgClass
        }
    }
}
</script>

<style lang="scss">

@import '@/assets/scss/skin.scss';

.blink-num {

    display: inline-block;
    width: 100%;
    box-sizing: border-box;
    animation: nanoBlink 1.1s 1;
    font-family: Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;
    word-wrap: break-word;
    text-align: right;


    &.green-color {
        color: $green
    }
    
    &.red-blink {
        animation: nanoRedBlink 1.1s 1;
    }

    &.green-blink {
        animation: nanoGreenBlink 1.1s 1;
    }
}


</style>