import { mapState } from 'vuex';
import { debounce } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

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
                        mergedVal += JSON.stringify(item[key] || '')
                    });
            

                return mergedVal.indexOf(this.searchKeywordDebounce || '') !== -1
            })
        },
    },
}