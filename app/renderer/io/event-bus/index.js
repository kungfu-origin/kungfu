import Vue from 'vue';
export const EVENT_BUS = new Vue();


export const onUpdateProcessStatusListener = (handler) => {
    EVENT_BUS.$on('update-process-status', handler)
}

export const offUpdateProcessStatusListener = (handler) => {
    EVENT_BUS.$off('update-process-status', handler)
}
