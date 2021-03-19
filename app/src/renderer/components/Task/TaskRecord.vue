<template>
    <tr-dashboard title="">
            <tr-table
            :data="taskRecords"
            :schema="tableHeader"
            :renderCellClass="renderCellClass"
            :keyField="tableKeyField"
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

        targetConfig () {
            return this.getTargetConfigByKey(this.currentConfigKey) || {};
        },

        tableHeader () {
            return this.targetConfig.displayConfig || this.getTaskDetailDefaultHeaders()
        },

        displayMode () {
            return this.targetConfig.displayMode
        },

        //这里要注意，因为tr-table的keyfield只允许str，所以如果多个key，需要构造一个id属性
        displayKeyMapKeys () {
            const keys = this.tableHeader
                .filter(item => item.key)
                .map(item => item.prop)

            if (keys.length) return keys
            else return ['updateTimestamp']
        },

        tableKeyField () {
            if (this.displayMode === 'keyMap') {
                return 'keyField'
            } else {
                return 'updateTimestamp'
            }
        }
    },

    methods: {

        dealTaskRecords (dataList) {

            const dataListResolved = Object.freeze(
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
                            updateTime: moment(value.updateTime).format('HH:mm:ss.SSS'),
                            updateTimestamp: record.update_time.toString(),
                            update: true
                        })
                    })
            )

            if (this.displayMode === 'keyMap') {
                let keyMap = {};
                dataListResolved.forEach(item => {
                    const key = this.displayKeyMapKeys.map(key => item[key]).join('_');
                    //因为此时record为时间最新在前，所以可以这么写
                    if (!keyMap[key]) {
                        keyMap[key] = Object.freeze({
                            ...item,
                            keyField: key
                        });
                    }
                })

                return Object.values(keyMap)
            }

            return dataListResolved
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