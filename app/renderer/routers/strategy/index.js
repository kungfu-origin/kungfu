// import Strategy from '@/components/Strategy/Index'
const Strategy = () => import(/* webpackPrefetch: true */'@/components/Strategy/Index')

export default [
    { path: '/strategy', name: 'strategy', component: Strategy }
]