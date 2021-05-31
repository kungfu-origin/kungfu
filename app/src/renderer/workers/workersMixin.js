
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
                    localStorage.setItem('instrumentsSavedDate', '')
                    return;
                }       
                
                if (this.getIfSaveInstruments(instruments)) {
                    if (!this.dealInstrumentController) {
                        this.dealInstrumentController = true;
                        Workers.dealInstruments.postMessage({
                            instruments: instruments
                        });
                    }
                }
            })


            Workers.dealInstruments.onmessage = debounce(function (event) {
                const { instruments } = event.data || {};
                localStorage.setItem('instrumentsSavedDate', moment().format('YYYY-MM-DD-HH-mm'))
                localStorage.setItem('instruments', JSON.stringify(instruments || []))
                this.oldInstruments = instruments || []; //refresh old instruments
                this.dealInstrumentController = false;
            })
        },

        getIfSaveInstruments (newInstruments) {
            if (newInstruments.length !== this.oldInstruments.length) {
                return true;
            }

            const instrumentsSavedDate = localStorage.getItem('instrumentsSavedDate')
            if (!instrumentsSavedDate) {
                return true
            } else if (instrumentsSavedDate !== moment().format('YYYY-MM-DD-HH-mm')) {
                return true 
            } else {
                return false
            }
        },
    }
}