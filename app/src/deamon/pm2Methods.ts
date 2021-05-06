
import { 

    watcher,

    dealGatewayStates, 
    transformTradingItemListToData, 
    transformOrderTradeListToData,
    transformOrderInputListToData, 
    transformOrderStatListToData, 
    transformAssetItemListToData,

    dealOrderInput,
    dealOrder,
    dealTrade,
    dealPos,
    dealAsset,
    dealOrderStat,
    dealSnapshot,
    dealQuote
} from '__io/kungfu/watcher';
import { ensureLeaderData } from '__gUtils/busiUtils';
const { _pm2 } = require('__gUtils/processUtils');

export const resLedgerData = (parentId: string, pm2Id: number, accountId: string, ticker: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const quotes = ensureLeaderData(ledger.Quote.filter('instrument_id', ticker)).map((item: QuoteOriginData) => dealQuote(item));
    const orders = getTargetOrdersByParentId(parentId);
    const positions = ensureLeaderData(ledger.Position).map((item: PosOriginData) => dealPos(item));
    const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];
    
    sendResDataToProcessId("LEDGER_DATA", pm2Id, processName, { 
        positions: positionsResolved, 
        quotes, 
        orders 
    })    
}

export const resQuoteData = (pm2Id: number, tickers: string, processName: string) => {
    if (!watcher.isLive()) return;
    
    watcher.step();
    const ledger = watcher.ledger;
    const quotes = ensureLeaderData(ledger.Quote)
        .filter((quote: QuoteOriginData) => tickers.includes(`${quote.instrument_id}_${quote.exchange_id}`))
        .map((quote: QuoteOriginData) => dealQuote(quote));

    sendResDataToProcessId("QUOTE_DATA", pm2Id, processName, { quotes })    
}

export const resInstrumentInfo = (pm2Id: number, tickers: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const instruments = ensureLeaderData(ledger.Instrument)
        .filter((item: InstrumentOriginData) => tickers.includes(`${item.instrument_id}_${item.exchange_id}`))
        
    sendResDataToProcessId('INSTRUMENT_DATA', pm2Id, processName, { instruments })
}

export const resPosData = (pm2Id: number, accountId: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const positions = ensureLeaderData(ledger.Position).map((item: PosOriginData) => dealPos(item));
    const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];

    sendResDataToProcessId("POS_DATA", pm2Id, processName, { 
        positions: positionsResolved,
    })
}

export const resOrderData = (pm2Id: number, parentId: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const orders = getTargetOrdersByParentId(parentId);
    
    sendResDataToProcessId("ORDER_DATA", pm2Id, processName, { 
        orders
    })
}

function getTargetOrdersByParentId (parentId: string) {
    const Order = watcher.ledger.Order;
    return ensureLeaderData(Order.filter('parent_id', BigInt(parentId))).map((item: OrderOriginData) => dealOrder(item))
}


function sendResDataToProcessId (topic: string, pm2Id: number, processName: string, data: any) {
    _pm2.sendDataToProcessId({
        type: 'process:msg',
        data,
        id: pm2Id,
        topic: topic
    }, (err: Error) => {
        if (err) {
            console.error(processName, err)
        }
    })
}