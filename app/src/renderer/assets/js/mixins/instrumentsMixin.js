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

    methods: {
        getInstrumentIds () {
            const instruments = localStorage.getItem('instruments')

            if (!instruments) {
                return Object.freeze([]);
            }
            
            return Object.freeze(JSON.parse(instruments))
        }
    },
}