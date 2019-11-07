<template>
<el-dialog
    custom-class="global-setting-dialog"
    width="90%" 
    title="全局设置"
    top="5vh"
    :visible="visible"
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleCancel"
    @keyup.enter.native="handleSubmitSetting"
>

    <div class="global-setting-content">
        <div class="dir-list">
            <el-collapse v-model="activeSettingTypes">
                <el-collapse-item :title="item.name" :name="item.key" v-for="item in settingConfig" :key="item.key">
                    <ul>
                        <li v-for="set in item.config" :key="set.key">{{set.name}}</li>
                    </ul>
                </el-collapse-item>
            </el-collapse>
        </div>
        <div class="setting-content">
                <div class="setting-type-item" v-for="setting in settingConfig" :key="setting.key">
                    <p class="setting-type-item__header" >{{setting.name}}</p>
                    <ul class="setting-type-item__content">
                        <li class="setting-item" v-for="item in Object.values(setting.config || {})" :key="item.key">
                            <p class="setting-item__header">{{item.name}}</p>

                            <div class="setting-sub-item__item" v-for="config in item.values" :key="config.key">
                                <p class="setting-sub-item__header">{{config.name}}</p>                                
                                <p class="setting-sub-item__tip">{{config.tip}}</p>
                                <div class="setting-sub-item__input-content">
                                    <!-- <el-input :class="item.key" v-if="item.type === 'str'" :type="item.key" :value="kfSystemConfig[item.key]" ></el-input>
                                    <el-input :class="item.key" v-if="item.type === 'password'" :type="item.key" :value="kfSystemConfig[item.key]" show-password></el-input>
                                    <el-switch :class="item.key" v-if="item.type === 'bool'" :value="kfSystemConfig[item.key]"></el-switch>
                                    <el-input-number :class="item.key" v-if="item.type === 'int'" :controls="false" :value="kfSystemConfig[item.key]"></el-input-number>
                                    <el-input-number :class="item.key" v-if="item.type === 'float'" :controls="false" :value="kfSystemConfig[item.key]"></el-input-number> -->
                                    <el-select :class="config.key" size="small" v-if="config.type === 'select'" :multiple="config.multiple" collapse-tags  :value="kfSystemConfig[item.key][config.key]" @input="e => handleIuput(item.key, config.key, e)">
                                        <el-option
                                            v-for="option in config.data"
                                            :key="option.value"
                                            :label="option.name"
                                            :value="option.value">
                                        </el-option>
                                    </el-select>
                                </div>
                            </div>
                            
                        </li>
                    </ul>    
                </div> 

        </div>
    </div>

</el-dialog>
</template>

<script>
import Vue from 'vue'
import { Collapse, CollapseItem } from 'element-ui';
import { readJsonSync, outputJson } from '__gUtils/fileUtils';
import { KF_CONFIG_PATH } from '__gConfig/pathConfig';

import systemConfig from '__gConfig/systemConfig.json';

const kfConfig = require(`${__resources}/config/kfConfig.json`) || {}
const path = require('path');

Vue.use(Collapse)
Vue.use(CollapseItem)

export default {
    name: 'global-setting-dialog',
    props: {
        visible: {
            type: Boolean,
            default: false
        },
    },

    data() {
        const t = this;
        return {
            kfSystemConfig: kfConfig,
            activeSettingTypes: ['system', 'trading'],
            settingConfig: {
                'system': { key: "system", name: '系统设置', config: systemConfig, value: kfConfig },
                'trading': { key: "trading", name: '交易设置', config: null, value: null }
            }
                
        }
    },

    mounted() {
        const t = this;
        t.settingConfig
    },

    methods: {

        handleCancel() {
            const t = this;
            t.close();
        },

        handleSubmitSetting() {},

        handleIuput(key, subKey, value) {
            const t = this;
            t.kfSystemConfig[key][subKey] = value;
            outputJson(KF_CONFIG_PATH, Object.freeze(t.kfSystemConfig || {}))
            .then(() => readJsonSync(KF_CONFIG_PATH))
            .then(kfConfig => t.kfSystemConfig = kfConfig)
        },


        close() {
            this.$emit('update:visible', false)
        },
    }
}

</script>

<style lang="scss">
@import "@/assets/scss/skin.scss";
    .global-setting-dialog {
        height: 90%;
        .el-dialog__body {
            height: calc(100% - 55px);
            box-sizing: border-box;
        }
        .global-setting-content {
            height: 100%;
        }
        .dir-list {
            width: 200px;
            display: inline-block;
            vertical-align: top;
            .el-collapse-item__content {
                li {
                    height: 30px;
                    line-height: 30px;
                    padding: 0 30px;
                    background: $bg_card;
                    cursor: pointer;
                }
                li:hover {
                    color: $font_5
                }
            }
        }
        .setting-content {
            display: inline-block;
            width: calc(100% - 300px);
            height: calc(100% - 55px);
            margin-left: 30px;

            .setting-type-item {
                padding-bottom: 30px;
            }

            .setting-type-item__header {
                font-size: 20px;
                font-weight: bold;
                color: $font;
                padding-bottom: 20px;
            }
            .setting-item__header {
                font-size: 18px;
                color: $font_5;
                padding-bottom: 15px;
            }
            .setting-sub-item__item {
                .setting-sub-item__header {
                    color: $vi;
                    font-size: 16px;
                    padding-bottom: 10px;
                }
                .setting-sub-item__tip {
                    font-size: 14px;
                    padding-bottom: 10px;
                }
                .setting-sub-item__input-content {
                    width: 300px;
                    padding-bottom: 30px;
                }
            }
            
        }
    }
</style>