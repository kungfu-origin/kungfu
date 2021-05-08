
import { 
    watcher,
    transformTradingItemListToData, 
    dealOrder,
    dealPos,
    dealQuote
} from '__io/kungfu/watcher';
import { ensureLeaderData } from '__gUtils/busiUtils';
const { sendDataToProcessIdByPm2 } = require('__gUtils/processUtils');

export const resLedgerData = (parentId: string, pm2Id: number, accountId: string, ticker: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const quotes = ensureLeaderData(ledger.Quote.filter('instrument_id', ticker)).map((item: QuoteOriginData) => dealQuote(item));
    const orders = getTargetOrdersByParentId(parentId);
    const positions = ensureLeaderData(ledger.Position).map((item: PosOriginData) => dealPos(item));
    const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];
    
    sendDataToProcessIdByPm2("LEDGER_DATA", pm2Id, processName, { 
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

    sendDataToProcessIdByPm2("QUOTE_DATA", pm2Id, processName, { quotes })    
}

export const resInstrumentInfo = (pm2Id: number, tickers: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const instruments = ensureLeaderData(ledger.Instrument)
        .filter((item: InstrumentOriginData) => tickers.includes(`${item.instrument_id}_${item.exchange_id}`))
        
    sendDataToProcessIdByPm2('INSTRUMENT_DATA', pm2Id, processName, { instruments })
}

export const resPosData = (pm2Id: number, accountId: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const ledger = watcher.ledger;
    const positions = ensureLeaderData(ledger.Position).map((item: PosOriginData) => dealPos(item));
    const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];

    sendDataToProcessIdByPm2("POS_DATA", pm2Id, processName, { 
        positions: positionsResolved,
    })
}

export const resOrderData = (pm2Id: number, parentId: string, processName: string) => {
    if (!watcher.isLive()) return;

    watcher.step();
    const orders = getTargetOrdersByParentId(parentId);
    
    sendDataToProcessIdByPm2("ORDER_DATA", pm2Id, processName, { 
        orders
    })
}

function getTargetOrdersByParentId (parentId: string) {
    const Order = watcher.ledger.Order;
    return ensureLeaderData(Order.filter('parent_id', BigInt(parentId))).map((item: OrderOriginData) => dealOrder(item))
}