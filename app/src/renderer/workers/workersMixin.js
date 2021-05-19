
import moment from 'moment';

import { debounce } from '__gUtils/busiUtils';
import { KF_DATASET_QUOTE_DIR } from '__gConfig/pathConfig';

import Workers from '@/workers/index';

export default {

    mounted () {
        this.bindAvgMarketVolumeListener();
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
        }
    }
}