import { KF_TICKER_SET_JSON_PATH } from '__gConfig/pathConfig';
import fse from 'fs-extra';
import { async } from 'rxjs';

interface TickerInTickerSet {
    source: string;
    exchangeId: string;
    instrumentId: string;
}

interface TickerSet {
    name: string;
    tickers: TickerInTickerSet[]
}

interface stringToTickerSet {
    [ propName: string ]: TickerSet;
}

export const getTickerSetsJSON = (): Promise<stringToTickerSet> => {
    return fse.readJSON(KF_TICKER_SET_JSON_PATH)
}

export const getTickerSets = (): Promise<TickerSet[]> => {
    return getTickerSetsJSON()
        .then(res => {
            console.log(res)
            return Object.values(res)
        })
}

export const addSetTickerSet = async (tickerSet: TickerSet) => {
    const tickerSetsJSON: stringToTickerSet = await getTickerSetsJSON();
    const name = tickerSet.name;
    if (!name) {
        throw new Error('No name in a tickerSet!')
    }
    tickerSetsJSON[name] = tickerSet;
    return fse.outputJson(KF_TICKER_SET_JSON_PATH, tickerSetsJSON)
}

export const removeTickerSetByName = async (targetName: string) => {
    const tickerSetsJSON: stringToTickerSet = await getTickerSetsJSON();
    if (tickerSetsJSON[targetName]) {
        delete tickerSetsJSON[targetName]
    }
    return fse.outputJson(KF_TICKER_SET_JSON_PATH, tickerSetsJSON)
}