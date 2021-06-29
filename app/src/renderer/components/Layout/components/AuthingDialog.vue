<template>
    <div class="kf-authing-dialog__warp" v-if="visible">
        <div class="kf-authing-dialog__mask" @click="handleClose"></div>
        <AuthingGuard :appId="AUTH_APPID" @login="handleLogin"></AuthingGuard>
    </div>
</template>

<script>
import { AuthingGuard, initAuthClient } from '@authing/vue-ui-components'
import '@authing/vue-ui-components/lib/index.min.css'

import { AuthenticationClient } from 'authing-js-sdk'

const AUTH_APPID = "60d974a92ad414330a2c6f8c";

initAuthClient({
    appId: AUTH_APPID,
})

export default {

    components: {
        AuthingGuard
    },

    props: {
        visible: Boolean
    },

    data () {

        this.AUTH_APPID = AUTH_APPID

        return {

        }
    },

    mounted () {
        this.initAuthing();
        const loginInfo = JSON.parse(localStorage.getItem('login_info') || "{}");
        this.$store.dispatch('setLoginInfo', Object.freeze(loginInfo));
    },

    methods: {

        handleLogin (res) {
            const loginInfo = Object.freeze(res);
            localStorage.setItem('login_info', JSON.stringify(loginInfo));
            this.$store.dispatch('setLoginInfo', loginInfo)
            this.handleClose();
        },

        initAuthing () {
            const authing = new AuthenticationClient({
                appId: AUTH_APPID,
                appHost: 'https://kungfu.authing.cn',
            })
            
            console.log("initAuthing", authing)
        },

        handleClose () {
            this.$emit("update:visible", false)
        },
    },

}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';

.kf-authing-dialog__mask {
    position: fixed;
    width: 100%;
    height: 100%;
    top: 0;
    left: 0;
    z-index: 99;
    opacity: .5;
    background: #000;
}

#authing_guard_container {
    position: fixed;
    left: 50%;
    top: 50%;
    transform: translateX(-50%) translateY(-50%);
    z-index: 101;
}

[class*=authing-].authing-guard-container {
    background: $bg_card !important;
}

[class*=authing-].authing-ant-avatar {
    border-radius: unset;
}

[class*=authing-].authing-tab-item__active:after {
    background-color: $vi;
}

[class*=authing-].authing-tab-item__active {
    color: $vi;
}

[class*=authing-].authing-ant-btn-primary {
    background: $vi;
    border-color: $vi;
}

[class*=authing-].authing-ant-btn-primary:focus, [class*=authing-].authing-ant-btn-primary:hover {
    background: $vi;
    border-color: $vi;
}

[class*=authing-].authing-guard-text-btn:focus, [class*=authing-].authing-guard-text-btn:hover {
    color: $vi;
}

[class*=authing-].authing-send-code-btn {
    color: $vi;
}

[class*=authing-].authing-guard-text-btn {
    color: $vi;
}


</style>