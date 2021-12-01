export enum StateStatusTypes {
    online = 'online',
    stopping = 'stopping',
    stopped = 'stopped',
    launching = 'launching',
    errored = 'errored',
    waitingrestart = 'waitingrestart',
    appState_0 = 0,
    appState_1 = 1,
    appState_2 = 2,
    appState_3 = 3,
    appState_4 = 4,
    appState_5 = 5,
    appState_100 = 100,
}

export type StateStatusColorTypes =
    | 'kf-color-running'
    | 'kf-color-waiting'
    | 'kf-color-error';

export interface StateStatusData {
    name: string;
    color: StateStatusColorTypes;
    level: number;
}

export interface TdOptions {
    accountName: string;
    accountId: string;
    sourceId: string;
    stateStatus: StateStatusTypes;
    processStatus: boolean;
    realizedPnl: number;
    unrealizedPnl: number;
    marketValue: number;
    margin: number;
    avail: number;
}
