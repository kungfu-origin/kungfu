import {
  DirectionEnum,
  InstrumentTypeEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';

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
    return price * volume * (exchange_rate || 1);
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
        (contract_multiplier || 1) *
        (long_margin_ratio || 1) *
        (exchange_rate || 1)
      );
    } else if (direction === DirectionEnum.Short) {
      return (
        price *
        volume *
        (contract_multiplier || 1) *
        (short_margin_ratio || 1) *
        (exchange_rate || 1)
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
