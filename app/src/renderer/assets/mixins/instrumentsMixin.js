import { mapGetters } from "vuex"

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
                const { instrument_id, instrument_name, exchange_id } = {
                        instrument_id: '',
                        instrument_name: '',
                        exchange_id: '',
                        ...item
                    }

                    if (`${instrument_id}${instrumentName}${exchange_id}`.toLowerCase().includes(queryString.toLowerCase())) return true;

                    const instrumentName = Buffer.from(instrument_name).toString();
                    if (instrumentName.toLowerCase().includes(queryString.toLowerCase())) return true;

                    return false
                })
                .slice(0, 300)
                .map(item => {
                    const { instrument_id, instrument_name, exchange_id } = {
                        instrument_id: '',
                        instrument_name: '',
                        exchange_id: '',
                        ...item
                    }
                    const instrumentName = Buffer.from(instrument_name).toString().split('\u0000').join('')

                    return {
                        instrument_id,
                        instrument_name: instrumentName,
                        exchange_id
                    }
                })
        },

        getSearchTickersInTickerSets (queryString = '') {
            return this.flatternTickers
                .filter((item => {
                    const { instrumentId, source, exchangeId } = item;
                    
                    return `${instrumentId}${exchangeId}${source}`.includes(queryString)
                }))
                .map(item => {
                    const { instrumentId, source, exchangeId } = item;

                    return {
                        instrument_id: instrumentId,
                        instrument_name: '',
                        exchange_id: exchangeId
                    }
                })
        }
    },
}