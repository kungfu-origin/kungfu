import fse from 'fs-extra';
import jschardet from 'jschardet';
import iconv from 'iconv-lite';
import { KF_INSTRUMENTS_PATH } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';

function decodeBuffer(name: number[]) {
  name = name.filter((n) => !!n);
  const bufferFrom = Buffer.from(name as unknown as ArrayBuffer);
  return isBufferGBK(bufferFrom)
    ? iconv.decode(bufferFrom, 'gbk')
    : iconv.decode(bufferFrom, 'utf8');
}

function isBufferGBK(bufferFrom: Buffer) {
  return jschardet.detect(bufferFrom).encoding !== 'UTF-8';
}

type InstrumentResolvedData = Record<string, KungfuApi.InstrumentResolved>;

const resolveInstruments = (
  existedInstruments: InstrumentResolvedData,
  instruments: KungfuApi.Instrument[],
): InstrumentResolvedData => {
  return (instruments || []).reduce((existedData, item) => {
    const { instrument_id, instrument_type, product_id, exchange_id, ukey } =
      item;
    const oldInstrument = existedData[ukey] || null;
    const instrumentName = decodeBuffer(product_id);
    const instrumentNameResolved =
      instrumentName || oldInstrument?.instrumentName || '';
    const instrumentType =
      instrument_type ||
      oldInstrument?.instrumentType ||
      InstrumentTypeEnum.unknown;

    const newInstrument: KungfuApi.InstrumentResolved = {
      instrumentId: instrument_id,
      instrumentType,
      instrumentName: instrumentNameResolved,
      exchangeId: exchange_id,
      id: `${instrument_id}_${instrumentNameResolved}_${exchange_id}`.toLowerCase(),
      ukey,
    };
    existedData[ukey] = newInstrument;
    return existedData;
  }, existedInstruments);
};

self.addEventListener('message', (e) => {
  const { instruments, tag } = e.data || {};

  if (tag === 'req_dealInstruments') {
    const existedInstruments: InstrumentResolvedData =
      fse.readJSONSync(KF_INSTRUMENTS_PATH);
    const newInstruments: InstrumentResolvedData = resolveInstruments(
      existedInstruments,
      instruments,
    );

    if (!Object.keys(newInstruments || {}).length) {
      return;
    }

    fse.outputJSONSync(KF_INSTRUMENTS_PATH, newInstruments);
    self.postMessage({
      updateTime: new Date().getTime(),
      instruments: Object.values(newInstruments),
    });
  }

  if (tag === 'req_instruments') {
    const instruments: InstrumentResolvedData =
      fse.readJSONSync(KF_INSTRUMENTS_PATH);
    self.postMessage({
      updateTime: new Date().getTime(),
      instruments: Object.values(instruments),
    });
  }
});
