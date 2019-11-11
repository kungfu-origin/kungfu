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
>

    <div class="global-setting-content">
        <div class="dir-list">
            <el-collapse v-model="activeSettingTypes" >
                <el-collapse-item :title="item.name" :name="item.key" v-for="item in settingConfig" :key="item.key">
                    <ul>
                        <li 
                        v-for="set in item.config" 
                        :key="set.key" 
                        :class="{'active': activeSettingItem === `${item.key}-${set.key}`}" 
                        @click="handleClickSettingType(item.key, set.key)">{{set.name}}</li>
                    </ul>
                </el-collapse-item>
            </el-collapse>
        </div>
        <div class="setting-content">
            <div class="setting-type-item" v-for="setting in settingConfig" :key="setting.key">
                <p class="setting-type-item__header" >{{setting.name}}</p>
                <ul class="setting-type-item__content">
                    <li class="setting-item" 
                        v-for="item in Object.values(setting.config || {})" 
                        :key="item.key"
                        :id="`setting-item-${setting.key}-${item.key}`"
                    >
                        <p class="setting-item__header">{{item.name}}</p>

                        <div class="setting-sub-item__item" v-for="config in item.config" :key="config.key">
                            <p class="setting-sub-item__header">{{config.name}}</p>                                
                            <p class="setting-sub-item__tip">{{config.tip}}</p>
                            <div class="setting-sub-item__input-content" v-if="settingConfig[setting.key].value">
                                <el-input 
                                :class="item.key" 
                                v-if="config.type === 'str'" 
                                :type="item.key" 
                                v-model="settingConfig[setting.key].value[item.key][config.key]" 
                                @blur="handleIuput(setting.key)" ></el-input>
                                <!-- <el-input :class="item.key" v-if="item.type === 'password'" :type="item.key" :value="kfSystemConfig[item.key]" show-password></el-input> -->
                                <el-switch 
                                :class="item.key" 
                                v-if="config.type === 'process'" 
                                :value="processStatus[config.target] === 'online'"
                                @change="e => handleSwitchProcess(e, config, settingConfig[setting.key].value[item.key])"
                                ></el-switch>
                                <span 
                                class="tr-oper" 
                                v-if="config.type === 'process'"                                 
                                @click.stop="handleOpenLogFile(config)"><i class="el-icon-document mouse-over"></i> 打开日志 </span>
                                
                                <!-- <el-input-number :class="item.key" v-if="item.type === 'int'" :controls="false" :value="kfSystemConfig[item.key]"></el-input-number> -->
                                <!-- <el-input-number :class="item.key" v-if="item.type === 'float'" :controls="false" :value="kfSystemConfig[item.key]"></el-input-number> -->
                                <el-select 
                                :class="config.key" 
                                v-if="config.type === 'select'" 
                                :multiple="config.multiple" collapse-tags  
                                v-model="settingConfig[setting.key].value[item.key][config.key]" 
                                @change="handleIuput(setting.key)" size="small">
                                    <el-option
                                        v-for="option in config.data"
                                        :key="option.value"
                                        :label="option.name"
                                        :value="option.value">
                                    </el-option>
                                </el-select>

                                <el-select 
                                :class="config.key" 
                                v-if="config.type === 'sources'" 
                                :multiple="config.multiple" collapse-tags  
                                v-model="settingConfig[setting.key].value[item.key][config.key]" 
                                @change="handleIuput(setting.key)" size="small">
                                    <el-option
                                        v-for="option in sourceList"
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
import { mapGetters, mapState } from 'vuex';
import { Collapse, CollapseItem } from 'element-ui';
import { readJsonSync, outputJson } from '__gUtils/fileUtils';
import { LOG_DIR, KF_CONFIG_PATH, KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { getExtensionConfigs, getExtensions } from '__gUtils/busiUtils';
import systemConfig from '__gConfig/systemConfig.json';
import tradingConfig from '__gConfig/tradingConfig.json';
import * as processUtils from '__gUtils/processUtils';

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
        const kfSystemConfig = readJsonSync(KF_CONFIG_PATH) || {}
        const kfTradingConfig = readJsonSync(KF_TARADING_CONFIG_PATH) || {}
        return {
            activeSettingTypes: ['system', 'trading'],
            activeSettingItem: '',
            settingConfig: {
                'system': { 
                    key: "system", 
                    name: '系统设置', 
                    config: systemConfig, 
                    value: kfSystemConfig,
                    outputPath: KF_CONFIG_PATH,
                    type: 'json'
                },
                'trading': { 
                    key: "trading", 
                    name: '交易设置', 
                    config: tradingConfig, 
                    value: kfTradingConfig,
                    outputPath: KF_TARADING_CONFIG_PATH,
                    type: 'json'
                }
            },

            sourceList: [],
                
        }
    },

    beforeMount() {
        const t = this;
        t.getSourceListOptions();
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus || {}
        })
    },

    methods: {
        handleClickSettingType(typeKey, itemKey) {
            const t = this;
            t.activeSettingItem = `${typeKey}-${itemKey}`;
            document.querySelector(`#setting-item-${t.activeSettingItem}`).scrollIntoView();
        },

        handleCancel() {
            const t = this;
            t.close();
        },

        handleIuput(settingKey) {
            const t = this;
            const settingData = t.settingConfig[settingKey].value;
            const outputPath = t.settingConfig[settingKey].outputPath;
            t.$nextTick()
            .then(() => outputJson(outputPath, settingData || {}))
            .then(() => readJsonSync(outputPath))
            .then(config => {
                if(!config) return;
                t.$set(t.settingConfig[settingKey], 'value', config)
            })
        },

        handleSwitchProcess(value, config, settingData) {
            const t = this;
            
            //开启
            if(value) {
                const startFn = processUtils[config.start];
                if (!(startFn instanceof Function)) return 
                const args = config.args;
                const params = args.map(arg => {
                    return settingData[arg]
                })
                startFn(config.target, ...params)
            } else {
                processUtils.deleteProcess(config.target)
            }
        },

        //打开日志
        handleOpenLogFile(config){
            const logPath = path.join(LOG_DIR, `${config.log_path}.log`);
            this.$showLog(logPath)
        },

        getSourceListOptions() {
            const t = this;
            getExtensionConfigs()
            .then((extList) => {
                return extList
                    .filter(e => e.type === 'source')
                    .map(e => ((e.config || {}).name || ''))
                    .filter(e => !!e)
                    .map(e => ({ name: e, value: e }))
            })
            .then(extOptions => t.sourceList = extOptions)
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
                li.active {
                    color: $vi;
                }
                li:hover {
                    color: $font_5
                }
                li:active {
                    color: $vi;
                }
            }
        }
        .setting-content {
            display: inline-block;
            width: calc(100% - 300px);
            height: calc(100% - 55px);
            margin-left: 30px;
            padding-bottom: 300px;
            box-sizing: border-box;
            overflow: auto;

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
            
            .tr-oper {
                padding-left: 20px;
                vertical-align: middle;
                i {
                    font-size: 14px !important;
                }
            }
        }
    }
</style>