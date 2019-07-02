<template>
    <span class="tr-dot-content">
        <span class="tr-dot" v-if="processName[value] " :class="processColor[value]"></span>
        {{hasText ? (processName[value] ? processName[value] : '--') : ''}}
    </span>
</template>

<script>
import {statusConfig} from '@/assets/config/statusConfig';
export default {
    name: 'tr-status',
    props: {
        value: '',
        hasText: {
            type: Boolean,
            default: true
        }
    },
    data() {
        let processName = {};
        let processColor = {};
        Object.keys(statusConfig || {}).map(key => {
            processName[key] = statusConfig[key].name
            processColor[key] = statusConfig[key].color

        })

        return {
            processName,
            processColor,
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.tr-dot-wave{
    width: 6px;
    height: 6px;
    display: inline-block;
    border-radius: 50%;
    vertical-align: middle;
    background: $green;
    position: relative;
    margin-right: 4px;
    &:after{
        position: absolute;
        top: -1px;
        left:-1px;
        width: 6px;
        height: 6px;
        border-radius: 50%;
        border: 1px solid $green;
        content: "";
        animation: antStatusProcessing 1s infinite ease-in-out;
    }
}

@keyframes antStatusProcessing
{
  0% {
    width: 6px;
    height: 6px;
    top: -1px;
    left:-1px;
    box-shadow: 0 0 0 rgba(98, 205, 153, 0.6);
    border: 1px solid rgba(98, 205, 153, 0.5);
  }
  100% {
    width: 10px;
    height: 10px;
    top: -3px;
    left:-3px;
    box-shadow: 0 0 6px rgb(98, 205, 153);
    border: 1px solid rgba(98, 205, 153, 0.1);
  }
}


</style>