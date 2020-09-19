import Vue from 'vue';
import VueRouter from 'vue-router';
import MakeOrderWindow from '@/components/Base/MakeOrderWindow';


Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/make-order', 
            name: 'MakeOrder', 
            component: MakeOrderWindow 
        },
        { path: '*', redirect: '/make-order' }
    ]
})
