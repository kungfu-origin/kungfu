import Vue from 'vue';
import VueRouter from 'vue-router';
import Admin from '../Index';

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/', 
            name: 'Admin', 
            component: Admin 
        },
        { path: '*', redirect: '/' }
    ]
})
