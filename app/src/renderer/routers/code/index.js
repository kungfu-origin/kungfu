// import Code from '@/components/Code/Index'
const Code = () => import('@/components/Code/Index')
export default [
    { path: '/kungfu-code-editor/:id', name: 'code', component: Code }
]