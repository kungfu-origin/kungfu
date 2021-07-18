import { getAuthClient, initAuthClient } from "@authing/vue-ui-components"
import { mapState } from 'vuex';

const AUTH_APPID = "60d974a92ad414330a2c6f8c";

initAuthClient({
    appId: AUTH_APPID,
    appHost: 'https://kungfu.authing.cn',
})


export default {

    data () {
        this.AUTH_APPID = AUTH_APPID;
        return {}
    },

    computed: {
        ...mapState({
            loginInfo: state => state.BASE.loginInfo || {}
        }),

        isActivated () {
            return this.loginInfo.status === "Activated" || false;
        },

        photo () {
            return this.loginInfo.photo || false
        },

        registerSource () {
            return this.loginInfo.registerSource || [];
        },

        name () {
            return this.loginInfo.name || this.loginInfo.nickname || '';
        },

        email () {
            return this.loginInfo.email || '';
        },

        phone () {
            return this.loginInfo.phone || '';
        },

        showName () {
            if (this.name) return this.name;

            //email
            if (this.loginInfo.registerSource.includes("basic:email")) {
                return this.email
            } else if (this.loginInfo.registerSource.includes("basic:phone-code")) {
                return this.phone 
            }
        }
    },

    methods: {
        openLoginInfoDialog () {
            this.$bus.$emit("open-login-info-dialog")
        },

        authLogout () {
            return getAuthClient()
                .logout()
                .then(() => {
                    this.$message.success('退出登陆成功！')
                })
                .catch(err => {
                    this.$message.error(err.message || "退出登陆失败！")
                })
                .finally(() => {
                    this.$store.dispatch('setLoginInfo', {})
                })

        },

        initAuthToken () {
            const loginInfo = JSON.parse(localStorage.getItem('login_info') || "{}");
            this.$store.dispatch('setLoginInfo', Object.freeze(loginInfo));
        },

        checkAuthToken () {
            const token = localStorage.getItem("login_token") || "";
            if (!token) return Promise.resolve(false)
            return getAuthClient()
                .checkLoginStatus(token)
                .then(res => {
                    const { status } = res;
                    if (!status) {
                        this.$store.dispatch('setLoginInfo', Object.freeze({}));
                        this.$message.info('登陆已过期')
                    }
                })
        },

        
    }
}