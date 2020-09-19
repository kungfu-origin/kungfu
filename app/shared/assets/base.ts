import { KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH, KF_FUTURE_TICKERS_CONFIG_PATH } from '__gConfig/pathConfig';
import { existsSync, addFileSync, readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils'

const fse = require('fs-extra');

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


interface FutureTickers {
    [ propName: string ]: {
        exchangeId: string,
        include: Array<{
            ticker: string,
            name: string
        }>
    }
}

export const getFutureTickersConfig = () => {
    return fse.readJson(KF_FUTURE_TICKERS_CONFIG_PATH)
        .then((res: FutureTickers) => {
            return reverseFutureTicker(res)
        })
}


interface FlatternFutureTicker {
    ticker: string;
    name: string;
    exchangeId: string;
}

function reverseFutureTicker (futureTickers: FutureTickers) {
    let flatternFutureTickers: Array<FlatternFutureTicker> = [];
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

