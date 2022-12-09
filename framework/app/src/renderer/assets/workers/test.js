const iconv = require('iconv-lite');
const jschardet = require('jschardet');

const defaultCharset = 'UTF-8';

const charsetTransformMap = {
  'UTF-8': 'utf8',
  GBK: 'gbk',
  GB2312: 'gb2312',
  GB18030: 'gb18030',
  Big5: 'big5',
};

function decodeBuffer(name) {
  const bufferFrom = Buffer.from(name);
  const charset = getBufferCharset(bufferFrom);

  return iconv.decode(bufferFrom, charset);
}

function getBufferCharset(buffer) {
  const { encoding, confidence } = jschardet.detect(buffer);
  console.log(encoding, confidence);
  if (confidence < 0.8) return defaultCharset;

  return encoding in charsetTransformMap
    ? charsetTransformMap[encoding]
    : encoding;
}

const a = [
  50, 49, -27, -88, -124, -23, -85, -104, 48, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 100, 59, -33, -1, -121, -61, 64, -4,
  -87, -15, -46, 77, 98, 80, 63, 0, 0, 0, 0, 0, 0, 89, 64, -4, -87, -15, -46,
  77, 98, 80, 63, 0, -31, -11, 5, 10, 0, 0, 0, -24, 3, 0, 0,
];

console.log(`${decodeBuffer(a)}`);
