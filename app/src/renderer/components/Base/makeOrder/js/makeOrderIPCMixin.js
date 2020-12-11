
import { mapState } from 'vuex';

import { ipcEmitDataByName } from '@/ipcMsg/emitter';
import { ipcRenderer } from 'electron';


export default {

    data () {

        return {
            currentId: '',
            moduleType: '',
            makeOrderByPosData: {}
        }
    },
    

    computed: {
        ...mapState({
            tdAccountSource: state => state.tdAccountSource || {},
            strategyList: state => state.strategyList,
            tdList: state => state.tdList,
            accountsAsset: state => state.accountsAsset,
        }),
    },

    methods: {
        cancelOrder (moduleType, orderData, strategyId) {
            return ipcEmitDataByName('cancelOrder', { moduleType, orderData, strategyId })
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            return ipcEmitDataByName('makeOrder', { moduleType, makeOrderForm, currentAccountResolved, strategyId })
        },

        init () {
            this.bindIPCOrderDataListener();
        },

        bindIPCOrderDataListener () {
            ipcRenderer.on('init-make-order-win-info', (event, info) => {
                const { currentId, makeOrderByPosData, moduleType } = info;                
                this.currentId = currentId;
                this.moduleType = moduleType;
                this.makeOrderByPosData = makeOrderByPosData;
            })
        },
    }
}