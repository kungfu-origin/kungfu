<template>
   <el-dialog title="选择柜台" 
        width="440px" 
        :visible="visible"
        id="select-source-dialog"
        @keyup.enter.native="handleSelectSource"
        @close="handleCloseSelectSource"
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        >
                <el-radio-group v-model.trim="selectedSource" style="width: 100%">
                    <el-row>
                        <el-col :span="12" v-for="item of Object.values(filteredAccountSource || {})" :key="item.source" :class="`source-${item.source}`">
                            <el-radio :label="item.source">
                                {{item.source.toUpperCase()}}
                                <el-tag
                                v-if="item.typeName"
                                :type="item.type" 
                                >
                                    {{(SourceTypeConfig[item.typeName] || {}).name || ''}}
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

import { SourceTypeConfig } from 'kungfu-shared/config/tradingConfig';
import { deepClone } from '__gUtils/busiUtils';

export default {
    props: {
        accountSource: {
            type: Object, 
            default: () => ({})
        },

        existMdList: {
            type: Array,
            default: () => []
        },

        visible: {
            type: Boolean,
            default: false
        },

    },

    data () {
        this.SourceTypeConfig = SourceTypeConfig;
        return {
            selectedSource: ''
        }
    },

    beforeMount () {
        this.ifNoAvailSource();
    },

    computed: {

        filteredAccountSource () {
            return this.buildSourceList();
        }
    },

    methods: {
        handleSelectSource () {
            if(!this.selectedSource) {
                this.$message.warning('还没有选择柜台！')
                return;
            };

            this.$emit('confirm', this.selectedSource)
            this.refreshData();
        },

        handleCloseSelectSource () {
            this.refreshData();
        },

        buildSourceList () {
            let accountSource = deepClone(this.accountSource);

            //only the md steps this process
            this.existMdList.forEach(md => {
                const source = md.source_name;
                if(accountSource[source]) delete accountSource[source]
            })

            return accountSource
        },

        ifNoAvailSource () {
            const accountSource = this.buildSourceList();
            if(Object.keys(accountSource || {}).length === 0) {
                this.$emit('noAvailSources', true)
                this.refreshData();
                return 
            }
        },

        refreshData () {
            this.selectedSource = ''
            this.$emit('update:visible', false);
        }
    }
}
</script>

<style lang="scss">

</style>