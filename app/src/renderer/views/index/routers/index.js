import Vue from 'vue';
import VueRouter from 'vue-router';
import StrategyRouter from './strategy';
import AccountRouter from './account';

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        ...StrategyRouter,
        ...AccountRouter,
        { path: '*', redirect: '/' }
    ]
})
