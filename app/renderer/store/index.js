import Vue from 'vue'
import Vuex from 'vuex'
import baseModule from './base';
import accountModule from './account';
import strategyModule from './strategy';


Vue.use(Vuex)

export default new Vuex.Store({
	strict: process.env.NODE_ENV !== 'production',
	modules: {
		BASE: baseModule,
		ACCOUNT: accountModule,
		STRATEGY: strategyModule
	}
})