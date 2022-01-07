import jschardet from 'jschardet';
import iconv from 'iconv-lite';

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

const resolveInstruments = (
    instruments: KungfuApi.Instrument[],
): KungfuApi.InstrumentResolved[] => {
    return (instruments || []).map((item) => {
        const {
            instrument_id,
            instrument_type,
            product_id,
            exchange_id,
            ukey,
        } = item;
        const instrumentName = decodeBuffer(product_id);
        return {
            instrumentId: instrument_id,
            instrumentType: instrument_type,
            instrumentName: instrumentName,
            exchangeId: exchange_id,
            id: `${instrument_id}_${instrumentName}_${exchange_id}`.toLowerCase(),
            ukey,
        };
    });
};

self.addEventListener('message', (e) => {
    const { instruments } = e.data || {};
    const instrumentsResolved = resolveInstruments(instruments);
    self.postMessage({
        updateTime: new Date().getTime(),
        instruments: instrumentsResolved,
    });
});
