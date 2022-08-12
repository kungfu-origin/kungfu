import fse from 'fs-extra';
const apiContent = fse.readFileSync('./strategy.md').toString();

var kungfuFunctions = [];
var kungfuKeywords = [];
var kungfuProperties = [];

const functionsAndProperties = apiContent.split('## 函数定义')[1];
const functionsMD = functionsAndProperties.split('## 常量定义')[0];
const propertiesMD = functionsAndProperties.split('## 常量定义')[1];

kungfuFunctions = functionsMD
  .split('####')
  .filter((f) => f.split('>')[0].indexOf('###') === -1)
  .map((f) => f.split('>')[0].trim())
  .filter((name) => name.indexOf('.') === -1);

const taurusKeywordsTmp = propertiesMD
  .split('###')
  .map((p) => {
    const re = />([\s\S]+?)</g;
    return p.match(re);
  })
  .filter((n) => !!n)
  .map((n) => n[0].split(' ')[0].slice(1));

kungfuKeywords = propertiesMD
  .split('\n')
  .filter((l) => l.includes('|'))
  .filter((l) => l.includes('---'))
  .map((l) => l.split('|')[1].trim())
  .filter((l) => {
    const re = /.*[\u4e00-\u9fa5]+.*/;
    return !re.test(l);
  });

const tarusPropertyKeywordsTmp = functionsMD
  .split('####')
  .filter((f) => f.split('>')[0].indexOf('###') === -1)
  .map((f) => f.split('>')[0].trim())
  .filter((name) => name.includes('.'))
  .map((n) => {
    return n.split('.').slice(1);
  })
  .reduce((a, b) => {
    return a.concat(b);
  });

kungfuProperties = tarusPropertyKeywordsTmp.concat(kungfuKeywords);
kungfuKeywords = kungfuProperties.concat(taurusKeywordsTmp);

fse.writeFile('./kungfuFunctions', '', () => {});
fse.writeFile('./kungfuKeywords', '', () => {});
fse.writeFile('./kungfuPropertys', '', () => {});

kungfuFunctions = Array.from(new Set(kungfuFunctions));
kungfuKeywords = Array.from(new Set(kungfuKeywords));
kungfuProperties = Array.from(new Set(kungfuProperties));

fse.appendFile(
  './kungfuFunctions',
  Array.from(new Set(kungfuFunctions)).join('\n'),
  () => {},
);
fse.appendFile(
  './kungfuKeywords',
  Array.from(new Set(kungfuKeywords)).join('\n'),
  () => {},
);
fse.appendFile(
  './kungfuProperties',
  Array.from(new Set(kungfuProperties)).join('\n'),
  () => {},
);
