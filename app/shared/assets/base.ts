import fse from 'fs-extra';
import { KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH, KF_FUTURE_TICKERS_CONFIG_PATH, KF_STOCK_TICKERS_CONFIG_PATH } from '__gConfig/pathConfig';
import { existsSync, addFileSync, readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils'

export const initConfig = () => {
    if(!existsSync(KF_CONFIG_PATH)) {
        addFileSync('', KF_CONFIG_PATH, 'file');
        const kfConfigJSON = readJsonSync(KF_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_CONFIG_PATH, kfConfigJSON)
    }

    if(!existsSync(KF_TARADING_CONFIG_PATH)) {
        addFileSync('', KF_TARADING_CONFIG_PATH, 'file');
        const kfTradingConfigJSON = readJsonSync(KF_TARADING_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_TARADING_CONFIG_PATH, kfTradingConfigJSON)
    }
};


interface Tickers {
    [ propName: string ]: {
        exchangeId: string,
        include: Array<{
            ticker: string,
            name: string
        }>
    }
}

interface FlatternTicker {
    ticker: string;
    name: string;
    exchangeId: string;
}

export const getFutureTickersConfig = () => {
    return fse.readJson(KF_FUTURE_TICKERS_CONFIG_PATH)
        .then((res: Tickers) => {
            return reverseFutureTicker(res)
        })
}

export const getStockTickersConfig = () => {
    return fse.readJson(KF_STOCK_TICKERS_CONFIG_PATH)
        .then((res: { [propName: string]: Array<FlatternTicker> }) => {
            return res.stock
        })
}




function reverseFutureTicker (futureTickers: Tickers) {
    let flatternFutureTickers: Array<FlatternTicker> = [];
    Object.values(futureTickers || {})
        .forEach(item => {
            const { exchangeId, include } = item;
            include.forEach((data: { ticker: string, name: string }) => {
                flatternFutureTickers.push({
                    ...data,
                    exchangeId
                })
            })
        })

    return flatternFutureTickers
}

