export const SET_TASKS = (state, tasks) => {
    state.tasks = tasks
}

export const SET_CALENDAR = (state, calendar) => {
    state.calendar = calendar
}

export const SET_TRADING_DAY = (state, tradingDay) => {
    state.calendar.trading_day = tradingDay
}

export const SET_PROCESS_STATUS = (state, processStatus) => {
    state.processStatus = processStatus
}

export const SET_ACCOUNT_SOURCE = (state, accountSource) => {
    state.accountSource = accountSource
}