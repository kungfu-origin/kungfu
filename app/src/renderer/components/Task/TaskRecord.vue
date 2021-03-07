<template>
    <tr-dashboard title="">
            <tr-table
            v-if="currentTaskId && tableHeader"
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
            const config = this.getTargetConfigByKey(this.currentConfigKey);
            return config.displayConfig || []
        }
    },

    methods: {
        getTargetConfigByKey (key) {
            return findTargetFromArray(this.taskExtConfigList, 'key', key)
        },
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