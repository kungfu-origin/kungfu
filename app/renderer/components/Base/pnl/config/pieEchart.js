export default {
    tooltip: {
        show: true,
        showContent: true,
        trigger: 'item',
        backgroundColor: 'rgba(44,52,80, 0.5)',
    },
    color: ['#F9A53E', '#E94840', '#3197D8'],
    // grid: {
    //     top: 50,
    //     containLabel: true //是否包含刻度标签
    // },
    series: [
        {
            type:'pie',
            radius: ['50%', '76%'],
            center: ['45%', '43%'],
            avoidLabelOverlap: false,
            label: {
                normal: {
                    show: false,
                    position: 'center'
                },
                emphasis: {
                    show: true,
                    textStyle: {
                        fontSize: '20',
                        fontWeight: 'bold'
                    }
                }
            },
            labelLine: {
                normal: {
                    show: false
                }
            },
            data:[] 
        }
    ]
}