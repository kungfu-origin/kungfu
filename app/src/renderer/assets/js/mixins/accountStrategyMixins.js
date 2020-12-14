
const remote = require('electron').remote

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
            } else if (this.moduleType === 'ticker'){
                return this.currentId
            } else {
                return ''
            }
        },
    },

    methods: {

        handleMakeOrderByPos (item) {
            this.makeOrderByPosData = Object.freeze(item);
            this.$nextTick().then(() => this.handleShowOrCloseMakeOrderDashboard());
        },

        handleShowOrCloseMakeOrderDashboard () {
            return this.buildMakeOrderWin()
                .then(() => {
                    this.emitCurrentMakeOrderWinInfo();
                    window.makeOrderWin.show && window.makeOrderWin.show();
                    window.makeOrderWin.focus && window.makeOrderWin.focus();
                })        
        },

        buildMakeOrderWin () {
            if (!window.makeOrderWin) {
                return this.$utils.openVueWin(
                    'makeOrder', 
                    `/make-order`, 
                    remote, 
                    { width: 470, height: 520 }
                ).then((win) => {
                    
                    window.makeOrderWin = win;
                    window.makeOrderWin.setAlwaysOnTop(true);
                    this.bindMakeOrderWinEvent();
                })
            } else {
                return Promise.resolve(true)
            }
        },

        emitCurrentMakeOrderWinInfo () {
            if (!window.makeOrderWin) return;
            
            window.makeOrderWin.webContents.send('init-make-order-win-info', {
                moduleType: this.moduleType,
                currentId: this.currentIdInAccountStrategyResolved,
                makeOrderByPosData: this.makeOrderByPosData,
                currentTicker: this.currentTicker
            })
        },

        bindMakeOrderWinEvent () {

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