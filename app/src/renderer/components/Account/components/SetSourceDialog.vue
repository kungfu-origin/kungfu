<template>
   <el-dialog title="选择柜台" 
            width="440px" 
            :visible.sync="visible"
            id="select-source-dialog"
            @keyup.enter.native="handleSelectSource"
            @close="handleCloseSelectSource"
            :close-on-click-modal="false"
            :close-on-press-escape="true"
            >
                    <el-radio-group v-model.trim="selectedSource" style="width: 100%">
                        <el-row>
                            <el-col :span="12" v-for="item of Object.values(accountSource || {})" :key="item.source" :class="`source-${item.source}`">
                                <el-radio :label="item.source">
                                    {{item.source.toUpperCase()}}
                                    <el-tag
                                    v-if="item.typeName"
                                    :type="item.type" 
                                    >
                                        {{(sourceTypeConfig[item.typeName] || {}).name || ''}}
                                    </el-tag>
                                </el-radio>
                            </el-col>
                        </el-row>
                    </el-radio-group>
                <div slot="footer" class="dialog-footer">
                    <el-button @click="handleCloseSelectSource" size="mini">取 消</el-button>
                    <el-button type="primary" size="mini" @click="handleSelectSource" id="confirm-select-source-btn">确 定</el-button>
                </div>
            </el-dialog>
</template>

<script>

import { sourceTypeConfig } from '__gConfig/tradingConfig';


export default {
    props: {
        accountSource: {
            type: Object, 
            default: () => ({})
        },

        visible: {
            type: Boolean,
            default: false
        },
        
    },

    data() {
        this.sourceTypeConfig = sourceTypeConfig;
        return {
            selectedSource: ''
        }
    },

    methods: {
        
        handleSelectSource() {
            const t = this;
            if(!t.selectedSource) {
                t.$message.warning('还没有选择柜台！')
                return;
            };

            t.$emit('confirm', t.selectedSource)
            t.refreshData();
        },

        handleCloseSelectSource() {
            const t = this;
            t.refreshData();
        },

        refreshData() {
            const t = this;
            t.$emit('update:visible', false);
            t.selectedSource = ''
        },

    }

}
</script>

<style lang="scss">

</style>