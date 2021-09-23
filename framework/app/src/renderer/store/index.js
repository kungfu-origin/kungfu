import Vue from 'vue'
import Vuex from 'vuex'
import baseModule from './base';
import accountModule from './account';
import strategyModule from './strategy';
import marketModule from './market';

Vue.use(Vuex)

export default new Vuex.Store({
	strict: process.env.NODE_ENV === 'development',
	modules: {
		BASE: baseModule,
		ACCOUNT: accountModule,
		STRATEGY: strategyModule,
		MARKET: marketModule,
	},
})