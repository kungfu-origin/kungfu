import moment from 'moment';

export const getCurrentCount = ({
    currentTimestamp, 
    deltaMilliSeconds,
    finishTime,
    LOOP_INTERVAL,
    LAST_SINGULARITY_SECOND,
    LAST_STEP_COUNT
}: {
    currentTimestamp: number; 
    deltaMilliSeconds: number;
    finishTime: number;
    LOOP_INTERVAL: number;
    LAST_SINGULARITY_SECOND: number;
    LAST_STEP_COUNT: number
}) => {
    const deltaCurrentToFinishTime = finishTime - currentTimestamp;
    const currentCount = Math.floor(deltaMilliSeconds / LOOP_INTERVAL)
    if (LAST_SINGULARITY_SECOND) {
        if (deltaCurrentToFinishTime <= LAST_SINGULARITY_SECOND) {
            return LAST_STEP_COUNT
        } else {
            if (currentCount === LAST_STEP_COUNT) {
                return currentCount - 1
            }
        }
    }
    return currentCount
};

export function getCurrentTimestamp (format = false): number | string {
    if (format) {
        return moment().format('HH:mm:ss.SSS')
    }
    return +new Date().getTime()
}