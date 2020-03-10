// import Code from '@/components/Code/Index'
const Code = () => import('@/components/Code/Index')
export default [
    { 
        path: '/kungfuCodeEditor/:id', 
        name: 'kungfuCodeEditor', 
        component: Code }
]