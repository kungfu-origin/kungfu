import { hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  DirectionEnum,
  InstrumentTypeEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

interface AccountingUsage {
  intrumentType: InstrumentTypeEnum;
  getTradeAmount: (
    watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) => number | null;
  getInstrumentInWatcher: (
    watcher: KungfuApi.Watcher,
    instrumentId: string,
    exchangeId: string,
  ) => KungfuApi.Instrument | null;
}

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

class BaseAccountingUsage implements AccountingUsage {
  intrumentType: InstrumentTypeEnum;
  constructor(intrumentType: InstrumentTypeEnum) {
    this.intrumentType = intrumentType;
  }

  getTradeAmount(
    _watcher: KungfuApi.Watcher,
    _instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ): number | null {
    throw Error('have to overwrite this function');
  }

  getInstrumentInWatcher(
    watcher: KungfuApi.Watcher,
    instrumentId: string,
    exchangeId: string,
  ) {
    const ukey = hashInstrumentUKey(instrumentId, exchangeId);
    return watcher.ledger.Instrument[ukey] as KungfuApi.Instrument | null;
  }
}

function calcTradeAmountWithNoting(
  instrumentForAccounting: KungfuApi.InstrumentForAccounting,
) {
  const { price, volume } = instrumentForAccounting;
  return price * volume;
}

class DefaultAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.unknown);
  }

  getTradeAmount(
    _watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    return calcTradeAmountWithNoting(instrumentForAccounting);
  }
}

function calcTradeAmountOnlyWithExchangeRate(
  instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  instrument: KungfuApi.Instrument | null,
) {
  const { price, volume } = instrumentForAccounting;
  const { exchange_rate } = instrument || {};
  return (
    price * volume * getInstrumentDefaultValue(exchange_rate, 'exchange_rate')
  );
}

class StockAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.stock);
  }

  getTradeAmount(
    watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    if (!instrumentForAccounting) return null;

    const { instrumentId, exchangeId } = instrumentForAccounting;
    return calcTradeAmountOnlyWithExchangeRate(
      instrumentForAccounting,
      this.getInstrumentInWatcher(watcher, instrumentId, exchangeId),
    );
  }
}

class BondAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.bond);
  }

  getTradeAmount(
    watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    if (!instrumentForAccounting) return null;

    const { instrumentId, exchangeId } = instrumentForAccounting;
    return calcTradeAmountOnlyWithExchangeRate(
      instrumentForAccounting,
      this.getInstrumentInWatcher(watcher, instrumentId, exchangeId),
    );
  }
}

class FutureAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.future);
  }

  getTradeAmount(
    watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    if (!instrumentForAccounting) return null;

    const { price, volume, direction, instrumentId, exchangeId } =
      instrumentForAccounting;

    const instrument = this.getInstrumentInWatcher(
      watcher,
      instrumentId,
      exchangeId,
    );

    const {
      contract_multiplier,
      long_margin_ratio,
      short_margin_ratio,
      exchange_rate,
    } = instrument || {};

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

class RepoAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.stock);
  }

  getTradeAmount(
    watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    const { volume, instrumentId, exchangeId } = instrumentForAccounting;

    const instrument = this.getInstrumentInWatcher(
      watcher,
      instrumentId,
      exchangeId,
    );

    const { exchange_rate } = instrument || {};
    return volume * getInstrumentDefaultValue(exchange_rate, 'exchange_rate');
  }
}

class CryptoAccountingUsage extends BaseAccountingUsage {
  constructor() {
    super(InstrumentTypeEnum.stock);
  }

  getTradeAmount(
    _watcher: KungfuApi.Watcher,
    instrumentForAccounting: KungfuApi.InstrumentForAccounting,
  ) {
    return calcTradeAmountWithNoting(instrumentForAccounting);
  }
}

const TradeAccountingUsageMap: Record<InstrumentTypeEnum, AccountingUsage> = {
  [InstrumentTypeEnum.unknown]: new DefaultAccountingUsage(),
  [InstrumentTypeEnum.stock]: new StockAccountingUsage(),
  [InstrumentTypeEnum.future]: new FutureAccountingUsage(),
  [InstrumentTypeEnum.fund]: new StockAccountingUsage(),
  [InstrumentTypeEnum.bond]: new BondAccountingUsage(),
  [InstrumentTypeEnum.stockoption]: new FutureAccountingUsage(),
  [InstrumentTypeEnum.warrant]: new FutureAccountingUsage(),
  [InstrumentTypeEnum.iopt]: new FutureAccountingUsage(),
  [InstrumentTypeEnum.techstock]: new StockAccountingUsage(),
  [InstrumentTypeEnum.index]: new StockAccountingUsage(),
  [InstrumentTypeEnum.repo]: new RepoAccountingUsage(),
  [InstrumentTypeEnum.crypto]: new CryptoAccountingUsage(),
  [InstrumentTypeEnum.cryptofuture]: new DefaultAccountingUsage(),
  [InstrumentTypeEnum.multi]: new DefaultAccountingUsage(),
};

export { TradeAccountingUsageMap };
