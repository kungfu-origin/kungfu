<template>
    <tr-dashboard title="">
            <tr-table
            :data="[]"
            :schema="tableHeader"
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
import { findTargetFromArray } from '__gUtils/busiUtils';


export default {

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
        getTargetConfigByKey (key) {
            return findTargetFromArray(this.taskExtConfigList, 'key', key)
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