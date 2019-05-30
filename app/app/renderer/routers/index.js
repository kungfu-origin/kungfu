import Vue from 'vue'
import VueRouter from 'vue-router'
import StrategyRouter from './strategy'
import AccountRouter from './account'
import CodeRouter from './code';


Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        ...StrategyRouter,
        ...AccountRouter,
        ...CodeRouter,
        {path: '*', redirect: '/'}
    ]
})
