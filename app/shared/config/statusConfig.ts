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
    '0':   { name: isEnglish ? 'Pending' : '等待中', color: 'bg-gray', level: -1 },
    '1':   { name: isEnglish ? 'Idle' : '无数据', color: 'bg-gray', level: 0 },
    '2':   { name: isEnglish ? 'Disconnected' : '已断开', color: 'bg-red', level: -1 },
    '3':   { name: isEnglish ? 'Connected' : '已连接', color: 'tr-dot-wave', level: 0 },
    '4':   { name: isEnglish ? 'LoggedIn' : '已登陆', color: 'bg-green', level: 1 },
    '5':   { name: isEnglish ? 'LoginFailed' : '登录失败', color: 'bg-red', level: -1 },
    '100': { name: isEnglish ? 'Ready' : '就绪',  color: 'bg-green', level: 1 }
}


