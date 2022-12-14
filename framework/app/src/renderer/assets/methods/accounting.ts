import {
  DirectionEnum,
  InstrumentTypeEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';

export const AccountingInstrumentDefaultValue = {
  contract_multiplier: 10,
  long_margin_ratio: 0.1,
  short_margin_ratio: 0.1,
  exchange_rate: 1,
};

export const getInstrumentDefaultValue = (
  value,
  key: keyof typeof AccountingInstrumentDefaultValue,
) => {
  return value || AccountingInstrumentDefaultValue[key];
};

class BaseTradeAmountUsage {
  constructor() {
    this;
  }

  getTradeAmount(...args): number | null {
    args;
    throw Error('have to overwrite this function');
  }

  getInstrumentInWathcer(exchangeId: string, instrumentId: string) {
    const instrumentKey = hashInstrumentUKey(instrumentId, exchangeId);

    const instrument = window.watcher.ledger.Instrument[
      instrumentKey
    ] as KungfuApi.Instrument | null;

    return instrument;
  }
}

class DefaultTradeAmountUsage extends BaseTradeAmountUsage {
  constructor() {
    super();
  }

  getTradeAmount(price: number, volume: number) {
    return price * volume;
  }
}

class StockTradeAmountUsage extends BaseTradeAmountUsage {
  constructor() {
    super();
  }

  getTradeAmount(
    price: number,
    volume: number,
    instrumentResolved: KungfuApi.InstrumentResolved,
  ) {
    const { exchangeId, instrumentId } = instrumentResolved;
    const { exchange_rate } =
      this.getInstrumentInWathcer(exchangeId, instrumentId) || {};
    return (
      price * volume * getInstrumentDefaultValue(exchange_rate, 'exchange_rate')
    );
  }
}

class FutureTradeAmountUsage extends BaseTradeAmountUsage {
  constructor() {
    super();
  }

  getTradeAmount(
    price: number,
    volume: number,
    direction: DirectionEnum,
    instrumentResolved: KungfuApi.InstrumentResolved,
  ) {
    if (!instrumentResolved) return null;

    const { exchangeId, instrumentId } = instrumentResolved;
    const {
      contract_multiplier,
      long_margin_ratio,
      short_margin_ratio,
      exchange_rate,
    } = this.getInstrumentInWathcer(exchangeId, instrumentId) || {};

    if (direction === DirectionEnum.Long) {
      return (
        price *
        volume *
        getInstrumentDefaultValue(contract_multiplier, 'contract_multiplier') *
        getInstrumentDefaultValue(long_margin_ratio, 'long_margin_ratio') *
        getInstrumentDefaultValue(exchange_rate, 'exchange_rate')
      );
    } else if (direction === DirectionEnum.Short) {
      return (
        price *
        volume *
        getInstrumentDefaultValue(contract_multiplier, 'contract_multiplier') *
        getInstrumentDefaultValue(short_margin_ratio, 'short_margin_ratio') *
        getInstrumentDefaultValue(exchange_rate, 'exchange_rate')
      );
    }

    return null;
  }
}

const TradeAmountUsageMap = {
  [InstrumentTypeEnum.unknown]: new DefaultTradeAmountUsage(),
  [InstrumentTypeEnum.stock]: new StockTradeAmountUsage(),
  [InstrumentTypeEnum.future]: new FutureTradeAmountUsage(),
};

export { TradeAmountUsageMap };
