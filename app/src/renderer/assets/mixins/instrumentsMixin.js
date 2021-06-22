import { mapGetters } from "vuex";

export default {
    data () {
        this.instrumentIds = this.getInstrumentIds()

        return {}
    },

    mounted () {
        this.$bus.$on('update:instruments', () => {
            this.instrumentIds = this.getInstrumentIds()
        })
    },

    beforeDestroy () {
        this.$bus.$off('update:instruments')
    },

    computed: {
        ...mapGetters([
            "flatternTickers"
        ])
    },

    methods: {
        getInstrumentIds () {
            const instruments = localStorage.getItem('instruments')
            if (!instruments) {
                return Object.freeze([]);
            }
            return Object.freeze(JSON.parse(instruments).map(item => Object.freeze(item)))
        },

        querySearch (queryString, cb) {
            const results = this.getSearchTickers(queryString);
            cb(results)
        },

        querySearchTickersInTickerSets (queryString, cb) {
            const results = this.getSearchTickersInTickerSets(queryString);
            cb(results)
        },

        getSearchTickers (queryString = '') {
            return this.instrumentIds
                .filter(item => {
                    const { id } = item;
                    if ((id || '').includes(queryString.toLowerCase())) return true;
                    return false
                })
                .slice(0, 20)
        },

        getSearchTickersInTickerSets (queryString = '') {
            const flatternTickers = this.flatternTickers
                .filter((item => {
                    const { instrumentId, exchangeId } = item;
                    return `${instrumentId}${exchangeId}`.toLowerCase().includes(queryString.toLowerCase())
                }))
                .map(item => {
                    const { instrumentId, exchangeId } = item;
                    return {
                        instrument_id: instrumentId,
                        instrument_name: '',
                        exchange_id: exchangeId
                    }
                })
            let noRepeatTickers = {};
            flatternTickers.forEach(ticker => {
                noRepeatTickers[`${ticker.instrument_id}_${ticker.exchange_id}`] = ticker;
            })

            return Object.values(noRepeatTickers)
        }
    },
}