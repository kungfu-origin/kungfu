
import moment from 'moment';

import { debounce } from '__gUtils/busiUtils';
import { KF_DATASET_QUOTE_DIR } from '__gConfig/pathConfig';
import { buildInstrumentsDataPipe } from '__io/kungfu/tradingData';

import Workers from '@/workers/index';

export default {

    data () {
        this.oldInstruments = Object.freeze(JSON.parse(localStorage.getItem('instruments') || "[]"));
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
                self.oldInstruments = instruments || []; //refresh old instruments
                self.dealInstrumentController = false;
            }
        },

        

        getIfSaveInstruments (newInstruments) {

            if (newInstruments.length !== this.oldInstruments.length) {
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