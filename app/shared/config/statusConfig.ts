
interface Status {
    name: string;
    color: string;
    level: number;
}

interface StatusConfig {
    [propName: string]: Status
}

export const statusConfig: StatusConfig = { 
    online: { name: '运行中', color: 'bg-green', level: 1 },
    stopping: { name: '停止中', color: 'bg-gray', level: 0 },
    stopped: { name: '已停止', color: 'bg-gray', level: 0 },
    launching: { name: '启动中', color: 'bg-gray', level: 0 },
    errored: { name: '错误', color: 'bg-red', level: -1 },
    '1': { name: '未启动', color: 'bg-gray', level: 0 },
    '2': { name: '断开', color: 'bg-red', level: -1 },
    '3': { name: '连接', color: 'tr-dot-wave', level: 1 },
    '4': { name: '登录就绪', color: 'bg-green', level: 1 },
    '5': { name: '登录失败', color: 'bg-red', level: -1 },
    '6': { name: '结算确认', color: 'bg-green', level: 1 },
    '7': { name: '结算失败', color: 'bg-red', level: -1 },
    '8': { name: '资金确认', color: 'bg-green', level: 1 },
    '9': { name: '资金错误', color: 'bg-red', level: -1 },
    '10': { name: '持仓确认', color: 'bg-green', level: 1 },
    '11': { name: '持仓错误', color: 'bg-red', level: -1 },
    '100': { name: '就绪',  color: 'bg-green', level: 1 }
}
