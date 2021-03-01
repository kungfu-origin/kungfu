/*
 * @Author: your name
 * @Date: 2020-06-10 13:39:45
 * @LastEditTime: 2020-06-22 13:55:58
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/src/renderer/views/index/routers/index.js
 */ 
import Vue from 'vue';
import VueRouter from 'vue-router';
import Account from '@/views/index/components/account/Index';
import MarketFilter from '@/views/index/components/marketFilter/Index';
import Strategy from '@/views/index/components/strategy/Index'
import MarketFilter from '@/views/index/components/marketFilter/Index'

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/',
            name: 'Account',
            component: Account
        },
        {
            path: '/market-filter',
            name: "MarketFilter",
            component: MarketFilter
        },
        { 
            path: '/strategy', 
            name: 'Strategy', 
            component: Strategy
        },
        { 
            path: '*', 
            redirect: '/' 
        }
    ]
})
