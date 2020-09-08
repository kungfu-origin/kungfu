export default {

    data () {
        
        return {
            makeOrderByPosData: {},
            showMakeOrderDashboard: false,
        }
    },

    methods: {
        handleShowOrCloseMakeOrderDashboard (val) {
            this.showMakeOrderDashboard = val
        },

        handleMakeOrderByPos (item) {
            this.showMakeOrderDashboard = true;
            this.$nextTick()
                .then(() => {
                    this.makeOrderByPosData = item
                })
        }
    }
}