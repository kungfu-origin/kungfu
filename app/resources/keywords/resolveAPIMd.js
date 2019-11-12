const fs = require('fs')
const apiContent = fs.readFileSync('./strategy.md').toString();

var kungfuFunctions = [];
var kungfuKeywords = [];
var kungfuProperties = [];


const functionsAndProperties = apiContent.split('## 函数定义')[1];
const functionsMD = functionsAndProperties.split('## 常量定义')[0];
const propertiesMD = functionsAndProperties.split('## 常量定义')[1];


kungfuFunctions = functionsMD
    .split('####')
    .filter(f => f.split('>')[0].indexOf('###') === -1)
    .map(f => f.split('>')[0].trim())
    .filter(name => name.indexOf('.') === -1)

taurusKeywordsTmp = propertiesMD
    .split('###')
    .map(p => {
        const re = />([\s\S]+?)</g;
        return p.match(re)
    })
    .filter(n => !!n)
    .map(n => n[0].split(' ')[0].slice(1))

kungfuKeywords = propertiesMD
    .split('\n')
    .filter(l => l.indexOf('|') !== -1)
    .filter(l => l.indexOf('---') === -1)
    .map(l => l.split('|')[1].trim())
    .filter(l => {
        const re = /.*[\u4e00-\u9fa5]+.*/;
        return !re.test(l)
    })

tarusPropertyKeywordsTmp = functionsMD
    .split('####')
    .filter(f => f.split('>')[0].indexOf('###') === -1)
    .map(f => f.split('>')[0].trim())
    .filter(name => name.indexOf('.') !== -1)
    .map(n => {
        return n.split('.').slice(1)
    })
    .reduce((a, b) => {
        return a.concat(b)
    })

    
kungfuProperties = tarusPropertyKeywordsTmp.concat(kungfuKeywords)
kungfuKeywords = kungfuProperties.concat(taurusKeywordsTmp)


fs.writeFile('./kungfuFunctions', '', () => {})
fs.writeFile('./kungfuKeywords', '', () => {})
fs.writeFile('./kungfuPropertys', '', () => {})

kungfuFunctions = Array.from(new Set(kungfuFunctions));
kungfuKeywords = Array.from(new Set(kungfuKeywords));
kungfuProperties = Array.from(new Set(kungfuProperties));

fs.appendFile('./kungfuFunctions', Array.from(new Set(kungfuFunctions)).join('\n'), () => {})
fs.appendFile('./kungfuKeywords', Array.from(new Set(kungfuKeywords)).join('\n'), () => {})
fs.appendFile('./kungfuProperties', Array.from(new Set(kungfuProperties)).join('\n'), () => {})


