
const BrowserWindow = require('electron').remote.BrowserWindow

export default {

    data () {

        return {
            makeOrderByPosData: {},
        }
    },



    mounted () {
        this.emitCurrentMakeOrderWinInfo();
    },

    watch: {
        currentIdInAccountStrategyResolved () {
            this.emitCurrentMakeOrderWinInfo();
        },  
    },

    computed: {
        currentIdInAccountStrategyResolved () {
            if (this.moduleType === 'strategy') {
                return this.strategyId
            } else if (this.moduleType === 'account') {
                return this.currentId
            } else {
                return ''
            }
        },
    },

    methods: {

        handleShowOrCloseMakeOrderDashboard () {

            if (this.moduleType === 'account') {
                if (!this.currentId) {
                    this.$message.warning(`需要先添加交易进程！`)
                    return;
                }
            }

            if (!window.makeOrderWin) {
                this.$utils.openVueWin(
                    'makeOrder', 
                    `/make-order`, 
                    BrowserWindow, 
                    {
                        width: 410,
                        height: 460
                    }
                ).then(({ win, curWinId }) => {
                    window.makeOrderWin = win;
                    window.makeOrderWin.faId = curWinId
                    window.makeOrderWin.setAlwaysOnTop(true);
                    this.bindMakeOrderWinEvent();
                })
            } else {
                this.emitCurrentMakeOrderWinInfo();
            }
            
            window.makeOrderWin.show && window.makeOrderWin.show();
            window.makeOrderWin.focus && window.makeOrderWin.focus();
        },

        handleMakeOrderByPos (item) {
            this.makeOrderByPosData = Object.freeze(item)
            this.$nextTick()
                .then(() => {
                    this.handleShowOrCloseMakeOrderDashboard();
                })
        },

        emitCurrentMakeOrderWinInfo () {
            if (!window.makeOrderWin) return;

            window.makeOrderWin.webContents.send('init-make-order-win-info', {
                moduleType: this.moduleType,
                currentId: this.currentIdInAccountStrategyResolved,
                makeOrderByPosData: this.makeOrderByPosData
            })
        },

        bindMakeOrderWinEvent () {

            this.emitCurrentMakeOrderWinInfo();


            window.makeOrderWin.on('show', () => {
                this.emitCurrentMakeOrderWinInfo();
            })
            
            window.makeOrderWin.on('close', () => {
                window.makeOrderWin = null;
                this.makeOrderByPosData = Object.freeze({})
            })
        },
        
     
    }
}