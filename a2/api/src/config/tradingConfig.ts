const isEnglish = false;

import {
    StateStatusData,
    StateStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings';

export const StateStatusConfig: Record<StateStatusTypes, StateStatusData> = {
    [StateStatusTypes.online]: {
        name: isEnglish ? 'Running' : '运行中',
        color: 'kf-color-running',
        level: 1,
    },
    [StateStatusTypes.stopping]: {
        name: isEnglish ? 'Stopping' : '停止中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.stopped]: {
        name: isEnglish ? 'Stopped' : '已停止',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.launching]: {
        name: isEnglish ? 'Launching' : '启动中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.errored]: {
        name: isEnglish ? 'Errored' : '错误',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusTypes.waitingrestart]: {
        name: isEnglish ? 'Stopped' : '待重启',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.appState_0]: {
        name: isEnglish ? 'Pending' : '等待中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.appState_1]: {
        name: isEnglish ? 'Idle' : '无数据',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.appState_2]: {
        name: isEnglish ? 'Disconnected' : '已断开',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusTypes.appState_3]: {
        name: isEnglish ? 'Connected' : '已连接',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.appState_4]: {
        name: isEnglish ? 'LoggedIn' : '已登录',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusTypes.appState_5]: {
        name: isEnglish ? 'LoginFailed' : '登录失败',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusTypes.appState_100]: {
        name: isEnglish ? 'Ready' : '就绪',
        color: 'kf-color-running',
        level: 1,
    },
};
