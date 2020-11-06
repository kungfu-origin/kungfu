import { mapGetters, mapState } from 'vuex';
import { setTimerPromiseTask } from '__gUtils/busiUtils';

export default {
    data () {

        const maunalClosedProcssList = JSON.parse(localStorage.getItem(`maunalClosedProcssList_${this.tdmdType}`) || '[]')

        return  {
            method: 'add',
            accountForm: {},
            selectedSource: '',
            visiblity: {
                selectSource: false,
                setAccount: false,
            },

            renderTable: false,

            keepAllProcessRunning: !!(+localStorage.getItem(`keepAllProcessRunning_${this.tdmdType}`)),
            
            maunalClosedProcssSet: new Set(maunalClosedProcssList)
        }
    },

    mounted () {
        this.renderTable = true;
        this.getTableList();
        this.keepProcessRunningLoopResolved()
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus || {}
        }),

        ...mapGetters([
            "ifMasterLedgerRunning"
        ])
    },

    watch: {
        keepAllProcessRunning (keepAllProcessRunning) {
            if (keepAllProcessRunning) {
                this.keepProcessRunningLoop()
            } else {
                this.keepRunningTimer && this.keepRunningTimer.clearLoop()
            }
        },
        
    },

    methods: {
        handleToggleKeepAllProcessRunning () {
            if (this.keepAllProcessRunning) {
                localStorage.setItem(`keepAllProcessRunning_${this.tdmdType}`, 0)
            } else {
                localStorage.setItem(`keepAllProcessRunning_${this.tdmdType}`, 1)
            }

            this.keepAllProcessRunning = !this.keepAllProcessRunning
        },

        //添加账户，打开选择柜台弹窗
        handleAdd () {
            this.visiblity.selectSource = true;
        },

        //编辑账户
        handleOpenUpdateAccountDialog (row) {
            this.method = 'update'
            this.accountForm = JSON.parse(row.config) 
            this.selectedSource = row.source_name
            this.visiblity.setAccount = true
        },
        //选择柜台
        handleSelectSource (sourceName) {
            this.method = 'add';
            this.selectedSource = sourceName
            this.visiblity.setAccount = true
        },

        keepProcessRunningLoopResolved () {
            if (this.keepAllProcessRunning) {
                this.keepProcessRunningLoop()
            } else {
                this.keepRunningTimer && this.keepRunningTimer.clearLoop()
            }
        },

        keepProcessRunningLoop () {
            if (this.switchAllProcess) {
                this.keepRunningTimer = setTimerPromiseTask(this.switchAllProcess, 3000)
            }
        },

        insertMaunalClosedProcssSet (id, status) {
            if (status) {
                this.maunalClosedProcssSet.delete(id)
            } else {
                this.maunalClosedProcssSet.add(id)
            }

            localStorage.setItem(`maunalClosedProcssList_${this.tdmdType}`, JSON.stringify(Array.from(this.maunalClosedProcssSet)))
        },

        //添加或修改账户详情后的操作
        successSubmitSetting () {
            this.getTableList();
            this.refreshData();
        },

        //清空数据
        refreshData () {
            this.selectedSource = ''
            this.accountForm = {};
        },
    }
    
}