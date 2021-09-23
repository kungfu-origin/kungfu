import moment from 'moment';

import { debounce } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { KF_DATASET_QUOTE_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { buildInstrumentsDataPipe } from '@kungfu-trader/kungfu-uicc/io/kungfu/tradingData';

import Workers from '@workers/index';

export default {

    data () {
        this.oldInstrumentsLength = JSON.parse(localStorage.getItem('instruments') || "[]").length;
        this.dealInstrumentController = false;
        return {}
    },

    mounted () {
        this.bindAvgMarketVolumeListener();
        this.bindDealInsturmentsListener();
    },

    methods: {
        bindAvgMarketVolumeListener () {
            const self = this;
            Workers.calcMarketDataAvgVolumeWorker.postMessage({
                days: 7,
                dataPath: KF_DATASET_QUOTE_DIR
            })

            Workers.calcMarketDataAvgVolumeWorker.onmessage = debounce(function (event) {{
                self.$store.dispatch('setMarketAvgVolume', event.data)
            }})
        },

        bindDealInsturmentsListener () {
            const self = this;

            buildInstrumentsDataPipe().subscribe(data => {
                const instruments = data['instruments'] || [];

                if (!instruments || !instruments.length) {
                    localStorage.setItem('instrumentsSavedDate', '');
                    return;
                }   
                
                if (self.getIfSaveInstruments(instruments) && !self.dealInstrumentController) {
                
                    self.dealInstrumentController = true;
                    console.time('DealInstruments')
                    console.log("DealInstruments postMessage", instruments.length)
                    Workers.dealInstruments.postMessage({
                        instruments: instruments
                    });
                }
            })


            Workers.dealInstruments.onmessage = event => {
                const { instruments } = event.data || {};
                console.timeEnd('DealInstruments')
                console.log("DealInstruments onmessage", instruments.length)
                localStorage.setItem('instrumentsSavedDate', moment().format('YYYY-MM-DD'))
                localStorage.setItem('instruments', JSON.stringify(instruments || []))
                self.oldInstrumentsLength = instruments.length || 0; //refresh old instruments
                self.dealInstrumentController = false;
            }
        },

        

        getIfSaveInstruments (newInstruments) {

            if (newInstruments.length !== this.oldInstrumentsLength) {
                return true;
            }

            const instrumentsSavedDate = localStorage.getItem('instrumentsSavedDate')
            if (!instrumentsSavedDate) {
                return true
            } else if (instrumentsSavedDate !== moment().format('YYYY-MM-DD')) {
                return true 
            } else {
                return false
            }
        },
    }
}