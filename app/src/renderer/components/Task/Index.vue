<template>
<tr-dashboard title="">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="handleAddTask" title="添加">添加</el-button>
        </tr-dashboard-header-item>
    </div>
    <div class="table-body">
        <el-table
        :data="tableListAfterFilter"
        size="small"
        height="100%"
        v-if="afterRender"
        >
            <el-table-column
                prop="processId"
                label="任务ID"
                sortable
                :show-overflow-tooltip="true"         
                >
            </el-table-column>
            <el-table-column
                label="状态"
                sortable  
                show-overflow-tooltip
            >
                <template slot-scope="props">
                    <tr-status 
                    v-if="$utils.ifProcessRunning(props.row.processId, processStatus)"
                    :value="props.row.processStatus"
                    ></tr-status>
                    <tr-status v-else></tr-status>
                </template>
            </el-table-column>
            <el-table-column
                label="运行"
                sortable    
            >
                <template slot-scope="props">
                    <el-switch
                    :value="$utils.ifProcessRunning(props.row.processId, processStatus)"
                    @change="e => handleTaskSwitch(e, props.row)"
                    ></el-switch> 
                </template>
            </el-table-column>
            <el-table-column
                label="" 
                width="120px"
                align="right"
            >
                <template slot-scope="props">
                    <span class="tr-oper-delete" @click.stop="handleDeleteTask(props.row)"><i class="mouse-over el-icon-delete"></i></span>
                </template>
            </el-table-column>
        </el-table>
    </div>

</tr-dashboard>

</template>
<script>

import { mapState } from 'vuex';

import baseMixin from '@/assets/js/mixins/baseMixin';

export default {

    name: 'task',
    
    mixins: [ baseMixin ],

    data () {

        return {

            searchFilterKey: 'processId'
        }
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus,
            processStatusWithDetail: state => state.BASE.processStatusWithDetail
        }),
    },

    watch: {
        processStatus (newProcess) {
            this.tableList = Object.keys(this.processStatusWithDetail || {})
                .map(key => {
                    return {
                        processId: key,
                        processStatus: newProcess[key].status
                    }
                })
                .filter(({ processId }) => {
                    if (processId.includes('task')) {
                        return true
                    }
                    return false
                })
        },

    },

    
    methods: {

        handleAddTask () {
            console.log(1111)
        },

        handleDeleteTask () {
            
        },

        handleTaskSwitch () {

        },
    }

}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';


</style>
