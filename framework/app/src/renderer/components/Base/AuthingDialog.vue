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
        @pwd-email-send-error="handleError"
        @pwd-phone-send-error="handleError"
        @close="handleClose"
        ></AuthingGuard>
    <!-- </div> -->
</template>

<script>
import { AuthingGuard, GuardMode } from '@authing/vue-ui-components'
import '@authing/vue-ui-components/lib/index.min.css'
import authMixin from '@/components/Base/js/authMixin';

export default {

    mixins: [ authMixin ],

    components: {
        AuthingGuard
    },

    props: {
        visible: Boolean
    },

    data () {
        this.GuardMode = GuardMode;

        return {}
    },

    methods: {

        handleLogin (res) {
            const loginInfo = Object.freeze(res);
            this.$store.dispatch('setLoginInfo', loginInfo)
            this.$message.success('登陆成功！');   
            this.handleClose();
        },

        handleRegister (res) {
            const loginInfo = Object.freeze(res);
            this.$store.dispatch('setLoginInfo', loginInfo);
            this.$message.success('注册成功！')
            this.handleClose();
        },

        handlePwdReset () {
            this.$message.success("密码修改成功！")
        },

        handleError(err) {
            console.log("Error from handleError")
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

// .authing-ant-message {
//     display: none;
// }

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

[class*=authing-].authing-ant-spin-dot-spin {
    color: $vi;
}

[class*=authing-].authing-ant-spin-dot-item {
    background-color: $vi;
}

[class*=authing-].authing-guard-complete-info-title {
    color: $icon;
}

[class*=authing-].authing-ant-form-item-label>label {
    color: $icon;
}

[class*=authing-].authing-guard-complete-info-msg {
    color: $font;
}

</style>