<template>
    <el-dialog 
    width="700px" 
    :title="`${method === 'add'? '添加' : '设置'} ${dialogTitle}`"  
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    :append-to-body="true"
    @close="handleClose"
    @keyup.enter.native="handleSubmitSetting"
    id="set-task-dialog"
    >
        <el-tabs tab-position="left" size="mini" v-model="activeTabName">
            <el-tab-pane
                :disabled="method !== 'add' && currentActiveConfigKey !== item.key"
                v-for="(item, index) in configListResolved"
                :key="item.key"
                :label="item.name"
                size="mini"
                :name="item.key"
                :lazy="true"
            >
                <ExtConfigForm
                    ref="taskSettingForm"
                    v-model="postFormList[index]"
                    :configList="item.config"
                    :uniKey="item.uniKey || ''"
                    labelWidth="140px"
                    :method="method"
                ></ExtConfigForm>
            </el-tab-pane>
        </el-tabs>        
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting" id="confirm-set-account-btn">确 定</el-button>
        </div>
     </el-dialog>
</template>

<script>

import moment from 'moment';
import ExtConfigForm from '@/components/Base/ExtConfigForm';

export default {

    props: {
        value: {
            type: Object,
            default: () => ({})
        },

        currentActiveConfigKey: '',

        configList: {
            type: Array,
            default: () => []
        },

        visible: {
            type: Boolean,
            default: false
        },

        method: {
            type: String,
            default: 'add'
        },

        outsideAddTaskType: {
            type: String,
            default: ''
        },

        outsideAddTaskInitData: {
            type: Object,
            default: () => ({})
        }
    },

    components: {
        ExtConfigForm
    },

    data () {
        return {
            activeTabName: this.getActiveTabNameByProps(),
            postFormList: this.getPostFormListByProps()
        }
        
    },

    computed: {

        dialogTitle () {
            if (this.outsideAddTaskType === 'mFilter') {
                return '行情任务'
            } else if (this.outsideAddTaskType === 'trade') {
                return '交易任务'
            } else {
                return '算法任务'
            }
        },

        configListResolved () {
            if (!this.outsideAddTaskType) {
                return this.configList
            } else {
                const afterFilerConfigList = this.configList.filter(item => item.subType === this.outsideAddTaskType)
                if (afterFilerConfigList.length) {
                    return afterFilerConfigList
                } else {
                    return this.configList
                }
            }
        },

        targetConfigIndex () {
            return this.configList.findIndex(item => item.key === this.activeTabName)
        },
    },

    methods: {
        handleClose () {
            this.$emit('update:visible', false);
            this.refreshData();
        },

        handleSubmitSetting () {
            this.$refs.taskSettingForm[this.targetConfigIndex].validate(valid => {
                if (valid) {
                    const postData = this.postFormList[this.targetConfigIndex]
                    this.$emit('confirm', JSON.stringify({
                        ...postData,
                        parentId: BigInt(+moment().valueOf()).toString(),
                        configKey: this.activeTabName,
                        subType: (this.configList[this.targetConfigIndex] || {}).subType || ''
                    }), this.activeTabName)
                    this.handleClose();
                }
            })
        },

        //从外部传入需要自动填写字段，根据column.type
        resolveOutsideInput (config, value = {}) {
            (config.config || [])
                .forEach(item => {
                    if (this.outsideAddTaskInitData[item.type]) {
                        value = {
                            ...value,
                            [item.key]: this.outsideAddTaskInitData[item.type]
                        }
                    }
                })
            return value
        },

        getActiveTabNameByProps () {
            const defaultKey = (this.configList[0] || {}).key || '';
            return this.currentActiveConfigKey ? this.currentActiveConfigKey : defaultKey
        },

        getPostFormListByProps () {
            const configIndexByKey = this.getConfigIndexByKey(this.currentActiveConfigKey);
            return this.configList.map((c, i) => {
                if (i === configIndexByKey) return this.value;
                return this.resolveOutsideInput(c, this.value); //只有在添加的时候
            });
        },

        getConfigIndexByKey (key) {
            return this.configList.findIndex(c => c.key === key)
        },

        refreshData () {
            this.postForm = {};
        },
    }
}
</script>

<style lang="scss">
    
</style>