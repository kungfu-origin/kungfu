
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

            if (!window.makeOrderWin) {
                window.makeOrderWin = this.$utils.openVueWin(
                    'makeOrder', 
                    `/make-order`, 
                    BrowserWindow, 
                    {
                        width: 410,
                        height: 460
                    }
                )

                this.bindMakeOrderWinEvent();
            } 
            
            window.makeOrderWin.show && window.makeOrderWin.show();
            window.makeOrderWin.focus && window.makeOrderWin.focus();

            this.$nextTick()
                .then(() => {
                    this.emitCurrentMakeOrderWinInfo();
                })
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
            window.makeOrderWin.webContents.on('did-finish-load', () => {
                this.emitCurrentMakeOrderWinInfo();
            })

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