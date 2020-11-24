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
import Strategy from '@/components/Strategy/Index'

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
            path: '/strategy', 
            name: 'strategy', 
            component: Strategy
        },
        { 
            path: '*', 
            redirect: '/' 
        }
    ]
})
