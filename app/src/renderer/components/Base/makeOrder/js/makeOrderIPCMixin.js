
import { mapState, mapGetters } from 'vuex';

import { ipcEmitDataByName } from '@/ipcMsg/emitter';
import { ipcRenderer } from 'electron';


export default {

    data () {

        return {
            currentId: '',
            moduleType: '',
            currentTicker: {},
            makeOrderByPosData: {},
            makeOrderByQuote: {}
        }
    },


    computed: {
        ...mapState({
            tdAccountSource: state => state.tdAccountSource || {},
            strategyList: state => state.strategyList,
            tdList: state => state.tdList,
            accountsAsset: state => state.accountsAsset,
        }),

        ...mapGetters([
            "proMode"
        ]),
    },

    methods: {
        cancelOrder (moduleType, orderData, strategyId) {
            return ipcEmitDataByName('cancelOrder', { moduleType, orderData, strategyId })
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            return ipcEmitDataByName('makeOrder', { moduleType, makeOrderForm, currentAccountResolved, strategyId })
        },

        subscribeTicker (sourceName, exchangeId, ticker) {
            return Promise.resolve(true)
        },

        init () {
            this.bindIPCOrderDataListener();
        },

        bindIPCOrderDataListener () {
            ipcRenderer.on('init-make-order-win-info', (event, info) => {
                const { currentId, makeOrderByPosData, moduleType, currentTicker } = info;                
                this.currentId = currentId;
                this.moduleType = moduleType;
                this.makeOrderByPosData = makeOrderByPosData;
                this.currentTicker = currentTicker;
            })
        },
    }
}