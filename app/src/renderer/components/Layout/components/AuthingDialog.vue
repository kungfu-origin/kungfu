<template>
    <!-- <div class="kf-authing-dialog__warp" v-if="visible"> -->
        <AuthingGuard 
        :visible.sync="visible"
        :appId="AUTH_APPID" 
        :config="{
            isSSO: true,
            escCloseable: true,
            clickCloseable: true,
            mode: GuardMode.Modal
        }"
        @login="handleLogin"
        @login-error="handleError"
        @register="handleRegister"
        @register-error="handleError"
        @pwd-reset="handlePwdReset"
        @pwd-reset-error="handleError"
        @close="handleClose"
        ></AuthingGuard>
    <!-- </div> -->
</template>

<script>
import { AuthingGuard, initAuthClient, GuardMode } from '@authing/vue-ui-components'
import '@authing/vue-ui-components/lib/index.min.css'


const AUTH_APPID = "60d974a92ad414330a2c6f8c";

initAuthClient({
    appId: AUTH_APPID,
    appHost: 'https://kungfu.authing.cn',
})

export default {

    components: {
        AuthingGuard
    },

    props: {
        visible: Boolean
    },

    data () {

        this.AUTH_APPID = AUTH_APPID;
        this.GuardMode = GuardMode;

        return {

        }
    },

    mounted () {
        const loginInfo = JSON.parse(localStorage.getItem('login_info') || "{}");
        this.$store.dispatch('setLoginInfo', Object.freeze(loginInfo));
    },

    methods: {

        handleLogin (res) {
            const loginInfo = Object.freeze(res);
            this.$store.dispatch('setLoginInfo', loginInfo)
            this.handleClose();
            this.$message.success('登陆成功！')
        },

        handleRegister (res) {
            const loginInfo = Object.freeze(res);
            this.$store.dispatch('setLoginInfo', loginInfo);
            this.handleClose();
            this.$message.success('注册成功！')
        },

        handlePwdReset (res) {
            this.$message.success("密码修改成功！")
        },

        handleError(err) {
            this.$store.dispatch('setLoginInfo', {})
            this.$message.error(err.message)
        },

        handleClose () {
            this.$emit("update:visible", false)
        },
    },

}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';

// #authing_guard_container {
//     position: fixed;
//     left: 50%;
//     top: 50%;
//     transform: translateX(-50%) translateY(-50%);
//     z-index: 101;
// }

.authing-ant-message {
    display: none;
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

[class*=authing-].authing-ant-btn-background-ghost.authing-ant-btn-primary {
    color: $vi;
    border-color: $vi;
}

[class*=authing-].authing-ant-btn-background-ghost.authing-ant-btn-primary:focus, [class*=authing-].authing-ant-btn-background-ghost.authing-ant-btn-primary:hover {
    color: $vi;
    border-color: $vi;
}

[class*=authing-].authing-guard-container h3 {
    color: $vi !important;
}

[class*=authing-].authing-guard-close-btn {
    background: transparent;
}

[class*=authing-].authing-guard-close-btn:hover {
    color: $vi;
}


</style>