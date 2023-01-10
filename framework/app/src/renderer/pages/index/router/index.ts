import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router';
import Index from '@kungfu-trader/kungfu-app/src/renderer/pages/index/views/Index.vue';

const routes: Array<RouteRecordRaw> = [
  {
    path: '/',
    name: 'Index',
    component: Index,
  },
];

const router = createRouter({
  history: createWebHashHistory(),
  routes,
});

export default router;
