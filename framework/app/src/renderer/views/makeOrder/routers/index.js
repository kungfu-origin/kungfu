import Vue from 'vue';
import VueRouter from 'vue-router';
import MakeOrderDashboard from '__renderer/components/Base/makeOrder/MakeOrderDashboard';


Vue.use(VueRouter)

export default new VueRouter({
    mode: 'hash',
    routes: [
        { 
            path: '/make-order', 
            name: 'MakeOrder', 
            component: MakeOrderDashboard 
        },
        { path: '*', redirect: '/make-order' }
    ]
})
