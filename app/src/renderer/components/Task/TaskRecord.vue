<template>
    <tr-dashboard title="">
            <tr-table
            :data="taskRecords"
            :schema="tableHeader"
            :renderCellClass="renderCellClass"
            keyField="updateTimestamp"
            @dbclickRow="() => {}"
            @clickCell="() => {}"
            @rightClickRow="() => {}"
            >
            </tr-table>
    </tr-dashboard>
</template>

<script>

import { mapState, mapGetters } from 'vuex';
import minimist from 'minimist';
import moment from 'moment';

import { findTargetFromArray, getDefaultRenderCellClass } from '__gUtils/busiUtils';
import { buildTaskDataPipe } from '__io/kungfu/tradingData';


export default {

    data () {

        this.taskDataPipe = null;
        return {
            taskRecords: [],
        }
    },

    mounted () {
        this.taskDataPipe = buildTaskDataPipe()
            .subscribe(dataList => {
                this.taskRecords = this.dealTaskRecords(dataList);
            })
    },

    destroyed () {
        this.taskDataPipe && this.taskDataPipe.unsubscribe();
    },

    computed: {
        ...mapState({
            taskExtConfigList: state => state.BASE.taskExtConfigList,
            currentTask: state => state.BASE.currentTask,
            currentTaskId: state => (state.BASE.currentTask).name
        }),

        ...mapGetters([
            "taskExtMinimistConfig"
        ]),

        currentConfigKey () {
            return minimist(this.currentTask.args || '', this.taskExtMinimistConfig).configKey || ''
        },

        tableHeader () {
            const config = this.getTargetConfigByKey(this.currentConfigKey) || {};
            return config.displayConfig || this.getTaskDetailDefaultHeaders()
        }
    },

    methods: {

        dealTaskRecords (dataList) {

            return Object.freeze(
                dataList
                    .filter(record => {
                        const { tag_a, tag_c } = record;
                        if (tag_a !== this.currentTaskId) return false;
                        if (tag_c !== 'task') return false;
                        return true;
                    })
                    .map(record => {
                        const value = JSON.parse(record.value || '{}');
                        return Object.freeze({
                            ...value,
                            id: record.update_time.toString(),
                            updateTime: moment(value.updateTime).format('HH:mm:ss'),
                            updateTimestamp: record.update_time.toString(),
                            update: true
                        })
                    })
            )
        },

        getTargetConfigByKey (key) {
            return findTargetFromArray(this.taskExtConfigList, 'key', key)
        },

        renderCellClass(prop, item) {   
            return getDefaultRenderCellClass(prop, item)
        },

        getTaskDetailDefaultHeaders () {
            return [{
                "type": "text",
                "label": "下单时间",
                "prop": "updateTime",
                "width": "70px"
            },{
                "type": "text",
                "label": "标的",
                "prop": "instrumentId",
                "width": "60px"
            },{
                "type": "text",
                "label": "",
                "prop": "side",
                "width": "40px"
            },{
                "type": "text",
                "label": "",
                "prop": "offset",
                "width": "40px"
            },{
                "type": "number",
                "label": "委托价",
                "prop": "limitPrice",
                "width": "80px"
            },{
                "type": "number",
                "label": "剩余时间(ms)",
                "prop": "remainTime",
                "width": "100px"
            },{
                "type": "text",
                "align": "center",
                "label": "下单量",
                "prop": "volume",
                "width": "60px"
            },{
                "type": "text",
                "align": "center",
                "label": "成交/目标量",
                "prop": "volumeTraded",
                "width": "90px"
            }]
        }
    }
}
</script>

<style lang="scss">

@import '@/assets/scss/skin.scss';

.kf-task-records__warp {

    width: 100%;
    height: 100%;
    background: $bg_card;

}

</style>