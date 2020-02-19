import * as actions from './actions';
import * as mutations from './mutations';
import * as getters from './getters';

export default {
    state: {
        order: {},
        trades: {},
        positions: {}
    },
    actions,
    mutations,
    getters
}
