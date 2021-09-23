import { mapState, mapGetters } from "vuex"
import minimist from 'minimist';
import moment from 'moment';

export default {
    computed: {
        ...mapState({
            taskExtConfigList: state => state.BASE.taskExtConfigList,
            processStatus: state => state.BASE.processStatus,
            processStatusWithDetail: state => state.BASE.processStatusWithDetail,
            currentTask: state => state.BASE.currentTask,
            currentTaskId: state => (state.BASE.currentTask).name
        }),

        ...mapGetters([
            "taskExtMinimistConfig"
        ]),

        taskList () {
            return Object.freeze(
                Object.keys(this.processStatusWithDetail || {})
                    .map(key => {
                        const targetProcess = this.processStatusWithDetail[key];
                        return this.buildTaskProcessItem(key, targetProcess)
                    })
                    .filter(({ processId }) => {
                        if (processId.includes('task')) {
                            return true
                        }
                        return false
                    })
            )

        }            
    },

    methods: {

        buildTaskProcessItem (key, pmData) {
            const argsConfig = minimist(pmData.args, this.taskExtMinimistConfig)

            return {
                processId: key,
                processStatus: pmData.status,
                createdAt: pmData.created_at ? moment(pmData.created_at).format('YYYY-MM-DD HH:mm:ss') : '--',
                configKey: argsConfig.configKey,
                subType: argsConfig.subType,
                ...pmData
            }
        },
    }
}