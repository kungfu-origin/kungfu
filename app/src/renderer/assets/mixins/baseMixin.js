import { debounce } from '__gUtils/busiUtils';

export default {

    data () {
        return {
            tableList: [],
            searchKeyword: "",
            searchKeywordDebounce: "",

            afterRender: false
        }
    },

    mounted () {
        this.afterRender = true;
    },

    beforeDestroy () {
        this.afterRender = false;
    },

    activated () {
        this.afterRender = true;
    },

    deactivated () {
        this.afterRender = false;
    },

    watch: {
        searchKeyword: debounce(function (value) {
            this.searchKeywordDebounce = value
        }),
    },

    computed: {
        tableListAfterFilter () {
            return this.tableList.filter(item => {
                if (!this.searchFilterKey) {
                    console.error(new Error('No searchFilterKey'))
                    return true;
                }

                let mergedVal = '';
                this.searchFilterKey
                    .split(',')
                    .map(key => key.trim())
                    .forEach(key => {
                        mergedVal += item[key]
                    });
            

                return mergedVal.includes(this.searchKeywordDebounce || '')
            })
        },
    },
}