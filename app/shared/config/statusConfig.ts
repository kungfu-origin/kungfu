const isEnglish = process.env.LANG_ENV === 'en';

interface Status {
    name: string;
    color: string;
    level: number;
}

interface StatusConfig {
    [propName: string]: Status
}

export const statusConfig: StatusConfig = { 
    'online':    { name: isEnglish ? 'Online' : '运行中', color: 'bg-green', level: 1 },
    'stopping':  { name: isEnglish ? 'Stopping' : '停止中', color: 'bg-gray', level: 0 },
    'stopped':   { name: isEnglish ? 'Stopped' : '已停止', color: 'bg-gray', level: 0 },
    'launching': { name: isEnglish ? 'Launching' : '启动中', color: 'bg-gray', level: 0 },
    'errored':   { name: isEnglish ? 'Errored' : '错误', color: 'bg-red', level: -1 },
    '1':   { name: isEnglish ? 'NoData' : '无数据', color: 'bg-gray', level: 0 },
    '2':   { name: isEnglish ? 'Disconnected' : '断开', color: 'bg-red', level: -1 },
    '3':   { name: isEnglish ? 'Connecting' : '连接中', color: 'tr-dot-wave', level: 0 },
    '4':   { name: isEnglish ? 'Ready' : '登录就绪', color: 'bg-green', level: 1 },
    '5':   { name: isEnglish ? 'LoginFailed' : '登录失败', color: 'bg-red', level: -1 },
    '6':   { name: isEnglish ? 'AccConfirmed' : '结算确认', color: 'bg-green', level: 0 },
    '7':   { name: isEnglish ? 'AccFailed' : '结算失败', color: 'bg-red', level: -1 },
    '8':   { name: isEnglish ? 'CashConfirmed' : '资金确认', color: 'bg-green', level: 0 },
    '9':   { name: isEnglish ? 'CashErrored' : '资金错误', color: 'bg-red', level: -1 },
    '10':  { name: isEnglish ? 'PosConfirmed' : '持仓确认', color: 'bg-green', level: 0 },
    '11':  { name: isEnglish ? 'PosErrored' : '持仓错误', color: 'bg-red', level: -1 },
    '100': { name: isEnglish ? 'Ready' : '就绪',  color: 'bg-green', level: 1 }
}
