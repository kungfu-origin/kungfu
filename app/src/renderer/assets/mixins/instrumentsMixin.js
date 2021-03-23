import { mapGetters } from "vuex";

export default {
    data () {
        this.instrumentIds = this.getInstrumentIds()

        return {}
    },

    mounted () {
        this.$bus.$off('update:instruments')
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
                    const { id } = item;
                    if ((id || '').includes(queryString.toLowerCase())) return true;
                    return false
                })
                .slice(0, 300)
        },

        decodeProductId (productId) {
            const productIdResolved = productId.filter(n => !!n);
            const bufferFrom = Buffer.from(productIdResolved);
            return this.isBufferGBK(bufferFrom) ? iconv.decode(bufferFrom, 'gbk') : iconv.decode(bufferFrom, 'utf8')
        },

        isBufferGBK (bufferFrom) {
            return jschardet.detect(bufferFrom).encoding !== 'UTF-8'
        },

        getSearchTickersInTickerSets (queryString = '') {
            return this.flatternTickers
                .filter((item => {
                    const { instrumentId, source, exchangeId } = item;
                    
                    return `${instrumentId}${exchangeId}${source}`.includes(queryString)
                }))
                .map(item => {
                    const { instrumentId, exchangeId } = item;
                    return {
                        instrument_id: instrumentId,
                        instrument_name: '',
                        exchange_id: exchangeId
                    }
                })
        }
    },
}