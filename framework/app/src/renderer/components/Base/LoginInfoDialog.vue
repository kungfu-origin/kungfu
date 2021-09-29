<template>
    <el-dialog
        custom-class="kf-login-info-dialog"
        width="70%"
        title="个人中心"
        top="15vh"
        :visible="visible"
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleCancel"
    >
        <div class="kf-login-info__warp">
            <div class="kf-login-info__header-content">
                <div class="header-image__content">
                    <div class="header-image__box" @click.stop="handleUploadHeadImage">
                        <img :src="this.photo" width="100%" height="100%" alt="">
                        <div class="upload-head-image-btn">修改</div>
                    </div>
                </div>
                <div class="header-info__content">
                    <div class="show-name">{{ showName }}</div>
                    <div class="info-box__content">
                        <div class="info-box">
                            <div class="label">手机号</div>
                            <div class="value">{{ phone || '-' }}</div>
                        </div>
                        <div class="info-box">
                            <div class="label">邮箱</div>
                            <div class="value">{{ email || '-' }}</div>
                        </div>
                    </div>
                </div>
                <div class="header-operation__content"></div>
            </div>
            <div class="kf-login-info__basic-content">
                <ExtConfigForm
                    ref="loginInfoForm"
                    v-model="formData"
                    :configList="loginInfoFormConfig || []"
                    label-width="100px"
                ></ExtConfigForm>
            </div>
        </div>
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleCancel" size="mini">取 消</el-button>
            <el-button  
            type="primary" 
            size="mini" 
            :loading="updateProfileLoading"
            @click="handleSaveLoginInfo" 
            >保 存</el-button>
        </div>
    </el-dialog>
</template>

<script>
import ExtConfigForm from '__renderer/components/Base/ExtConfigForm';

import { deepClone } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { getAuthClient } from "@authing/vue-ui-components"
import loginInfoFormConfig from '__renderer/components/Base/config/loginInfoFormConfig';


export default {

    props: {
        visible: Boolean,
        loginInfo: Object
    },

    data () {
        this.loginInfoFormConfig = loginInfoFormConfig;
        return {
            loginInfoClone: deepClone(this.loginInfo || {}),
            formData: this.buildFormData(this.loginInfo),
            updateProfileLoading: false
        }
    },

    components: {
        ExtConfigForm
    },

    watch: {
        loginInfo (newVal) {
            this.loginInfoClone = deepClone(newVal || {});
            this.formData = this.buildFormData(this.loginInfo)
        },
    },

    computed: {
        isActivated () {
            return this.loginInfoClone.status === "Activated" || false;
        },

        photo () {
            return this.loginInfoClone.photo || false
        },

        registerSource () {
            return this.loginInfoClone.registerSource || [];
        },

        name () {
            return this.loginInfoClone.name || '';
        },

        nickname () {
            return this.loginInfoClone.nickname || '';
        },

        email () {
            return this.loginInfoClone.email || '';
        },

        phone () {
            return this.loginInfoClone.phone || '';
        },

        showName () {
            if (this.nickname) return this.nickname;
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
        handleUploadHeadImage () {
            return getAuthClient()
                .uploadAvatar()
                .then(loginInfo => {
                    this.$store.dispatch("setLoginInfo", Object.freeze(loginInfo));
                    this.$message.success("更新头像成功！");
                })
                .catch(err => {
                    this.$message.error(err.message);
                })
        },

        handleCancel () {
            this.$emit("update:visible", false)
        },

        handleSaveLoginInfo () {
            this.updateProfile(this.buildPostData(this.formData))
        },

        buildFormData (loginInfo) {
            if (!loginInfo) return {};
            let formData = {};
            (loginInfoFormConfig || []).forEach(item => {
                const key = item.key;
                formData[key] = loginInfo[key];
            });
            return formData
        },

        buildPostData (formData) {
            if (!formData) return;
            let postData = {};
              (loginInfoFormConfig || [])
              .filter(item => item.type !== "readonly")
              .forEach(item => {
                const key = item.key;
                postData[key] = formData[key];
            });
            return postData
        },

        updateProfile (updates) {
            if (this.updateProfileLoading) return Promise.resolve(false);
            this.updateProfileLoading = true;
            return getAuthClient()
                .updateProfile(updates)
                .then(loginInfo => {
                    this.$store.dispatch("setLoginInfo", Object.freeze(loginInfo));
                    this.$message.success("保存个人信息成功！");
                })
                .catch(err => {
                    this.$message.error(err.message);
                })
                .finally(() => {
                    this.updateProfileLoading = false;
                })
        },
    }
}
</script>

<style lang="scss">

@import "__renderer/assets/scss/skin.scss";

.kf-login-info__warp {
    
    .kf-login-info__header-content {
        display: flex;
        height: 120px;
        border-bottom: 1px solid $bg_light;

        .header-image__content {
            width: 100px;
            text-align: center;

            .header-image__box {
                display: inline-block;
                width: 70px;
                height: 70px;
                border-radius: 50%;
                overflow: hidden;
                position: relative;

                &:hover {

                    .upload-head-image-btn {
                        display: block;
                    }
                }

                .upload-head-image-btn {
                    display: none;
                    width: 100%;
                    height: 20px;
                    line-height: 20px;
                    background: rgba($color: #000000, $alpha: 0.6);
                    color: $white;
                    font-size: 12px;
                    text-align: center;
                    position: absolute;
                    left: 0;
                    bottom: 0;
                }
            }
        }

        .header-info__content {
            padding: 0 20px;
            box-sizing: border-box;
            display: flex;
            flex-direction: column;

            .show-name {
                font-size: 24px;
                height: 32px;
                color: $icon;
                margin-bottom: 20px;
            }

            .info-box__content {

                .info-box {
                    width: 50%;
                    display: flex;
                    justify-content: flex-start;
                    margin-bottom: 10px;
                    
                    .label  {
                        width: 50px;
                        color: $font_1;
                    }

                    .value {
                        color: $font;
                    }
                }

            }
        }

        .header-info__content {
            flex: 1;
        }

        .header-operation__content {
            width: 160px;
        }
    }

    .kf-login-info__basic-content {
        padding-top: 30px;

        .el-form {
            display: flex;
            flex-wrap: wrap;

            .el-form-item {
                width: 50%;
            }
        }
    }
}

</style>