import Vue from 'vue';
import VueRouter from 'vue-router';
import Backtest from '@/components/Backtest/Index';

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/:id', 
            name: 'backtest', 
            component: Backtest 
        },

        { path: '*', redirect: '/' }
    ]
})
