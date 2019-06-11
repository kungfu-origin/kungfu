import {toDecimal} from '@/assets/js/utils';
export default {
    color: ['#53B9F8'],
    tooltip: {
        trigger: 'axis',
        backgroundColor: 'rgba(44,52,80, 1)',
        axisPointer: {
            type: 'cross',
            lineStyle: {
                color: '#2C3450',
                type: 'dashed'
            },
            crossStyle: {
                color: '#2C3450',
            }
        }
    },
    grid: {
        top: 30,
        left: 5,
        right: 5,
        bottom: 70,
        // containLabel: true //是否包含刻度标签
    },

    dataZoom: [
        {
            show: true,
            borderColor: '#2C3450',
            fillerColor: 'rgba(131,144,197,0.1)', //选中的蒙层颜色
            dataBackground: {
                areaStyle: { //填充的样式
                    color: 'black'
                }
            },
            textStyle: { //文字样式
                color: '#2C3450'
            }
        }

    ],

    xAxis: {
        type: 'category',
        data: [],
        axisLine: {      
            show: true,                                                                                         
            lineStyle: {
                color: '#2C3450'
            }
        },
        axisLabel: { 
            showMaxLabel: false,
            align: 'left',          
            margin: 5,
        },
        boundaryGap: false, //两边是否留白
    },
    yAxis: {
        // boundaryGap: ['0%', '5%'],
        type: 'value',
        axisLabel: {
            inside: true,                                                                                                
            showMaxLabel: false,                                              
            showMinLabel: false, 
            formatter: "{value}%",
            margin: 4 //刻度标签与轴线之间的距离
        },
        axisLine: {
            show: false,
            lineStyle: {
                color: '#2C3450'
            }
        },
        axisTick: {
            show: true,
            inside: true
        },
        splitLine: {
            show: false
        }
    },
    series: []
}