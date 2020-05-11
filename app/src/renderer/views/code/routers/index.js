import Vue from 'vue';
import VueRouter from 'vue-router';
import CodeRouter from './code';

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        ...CodeRouter,
        { path: '*', redirect: '/kungfuCodeEditor' }
    ]
})
