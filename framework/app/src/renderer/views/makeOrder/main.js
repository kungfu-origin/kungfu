
/* eslint-disable */
import Vue from 'vue';
import './setKungfuParamsOnWindow';
import store from './store';
import router from './routers';
import ElementUI from 'element-ui';
import Components from '__renderer/assets/components'
import App from './App.vue';

import '__renderer/assets/iconfont/iconfont.js';
import '__renderer/assets/iconfont/iconfont.css';
import '__renderer/assets/scss/makeOrder.scss';

Vue.use(ElementUI)

Vue.config.productionTip = false
Vue.store = Vue.prototype.$store = store
Vue.bus = Vue.prototype.$bus = new Vue();

//tr 组件
Vue.use(Components)


new Vue({
    router,
    store,
    render: h => h(App)
}).$mount('#app', true)


