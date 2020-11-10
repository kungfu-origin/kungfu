import Vue from 'vue';
import VueRouter from 'vue-router';
import Code from '../Index';

Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/kungfuCodeEditor/:id', 
            name: 'kungfuCodeEditor', 
            component: Code 
        },
        { path: '*', redirect: '/kungfuCodeEditor' }
    ]
})
