
import moment from 'moment';
import path from "path";
import { mapGetters, mapState } from 'vuex';

import { writeCSV } from '__gUtils/fileUtils';
import { KF_DATASET_QUOTE_DIR } from '__gConfig/pathConfig';
import { watcher } from '__io/kungfu/watcher';
import { delayMiliSeconds } from '__gUtils/busiUtils';

export default {

    data () {
        
        return {
            recordBeforeQuitLoading: false
        }
    },

    computed: {
        
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets
        }),

        ...mapGetters([
            "flatternTickers"
        ])
    },

    methods: {
        recordBeforeQuit () {
            this.recordBeforeQuitLoading = true;
            return Promise.all([
                this.recordQuote()
            ]).finally(() => {
                this.recordBeforeQuitLoading = false;
            })
        },

        recordQuote () {
            watcher.step()
            const quotes = Object.values(watcher.ledger.Quote || {});
            const tickerIds = this.flatternTickers.map(item => `${item.instrumentId}_${item.exchangeId}`).join(',')
            const subscribedQuotes = quotes
                .filter(item => {
                    return tickerIds.includes(`${item.instrument_id}_${item.exchange_id}`)
                })
                .map(item => {
                    return {
                        ...item,
                        instrument_id: item.instrument_id.toString()
                    }
                })
            
            if (!subscribedQuotes.length) {
                return Promise.resolve(false)
            }
            const fileName = moment().format('YYYY-MM-DD');
            return writeCSV(path.join(KF_DATASET_QUOTE_DIR, `${fileName}.csv`), subscribedQuotes)
        },
    }
}



