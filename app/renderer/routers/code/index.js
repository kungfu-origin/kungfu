// import Code from '@/components/Code/Index'
const Code = () => import('@/components/Code/Index')
export default [
    { path: '/code/:id', name: 'code', component: Code }
]