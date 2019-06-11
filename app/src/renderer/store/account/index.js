import * as actions from './actions';
import * as mutations from './mutations';
import * as getters from './getters';

export default {
    state: {
        mdTdState: {},
        accountsAsset: {},
        accountList: [],
        currentAccount: {},
    },
    actions,
    mutations,
    getters
}