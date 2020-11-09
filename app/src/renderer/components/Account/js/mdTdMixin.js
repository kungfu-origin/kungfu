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
        }
    },

    mounted () {
        this.renderTable = true;
        this.getTableList();
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus || {}
        }),

        ...mapGetters([
            "ifMasterLedgerRunning"
        ]),

        allProcessBtnTxt () {
            if (this.allProcessRunning) {
                return '关闭全部'
            } else {
                return '开启全部'
            }
        },
        
    },

    methods: {
        handleToggleKeepAllProcessRunning () {
            const targetStatus = !this.allProcessRunning;
            const allProcessBtnTxt = this.allProcessBtnTxt;

            timer && timer.clearLoop && timer.clearLoop();
            let count = 0;
            let timer = setTimerPromiseTask(() => {
                count++;

                if (!targetStatus) {
                    if (count > 3) {
                        timer && timer.clearLoop && timer.clearLoop();
                    }
                } else {
                    if (count > 3) {
                        timer && timer.clearLoop && timer.clearLoop();
                    }
                }

                //全部开启成功
                console.log(targetStatus, this.allProcessRunning)
                if (targetStatus === this.allProcessRunning) {
                    this.$message.success(`${allProcessBtnTxt} ${this.tdmdType} 进程 完成`)                
                    timer && timer.clearLoop && timer.clearLoop();
                    return Promise.resolve(false);
                }

                const loopTxt = count === 1 ? '' : `第${count}次`
                this.$message.info(`正在尝试 ${allProcessBtnTxt} ${this.tdmdType} 进程 ${loopTxt}`)                

                return this.switchAllProcess(targetStatus)
            }, 5000)

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