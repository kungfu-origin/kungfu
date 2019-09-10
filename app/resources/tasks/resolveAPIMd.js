const fs = require('fs')
const path = require('path')

const apiContent = fs.readFileSync('./strategyApi.md').toString();

var taurusDefKeywords = [];
var taurusKeywords = [];
var tarusPropertyKeywords = [];


const functionsAndProperties = apiContent.split('## 函数定义')[1];
const functionsMD = functionsAndProperties.split('## 常量定义')[0];
const propertiesMD = functionsAndProperties.split('## 常量定义')[1];


taurusDefKeywords = functionsMD
    .split('####')
    .filter(f => f.split('>')[0].indexOf('###') === -1)
    .map(f => f.split('>')[0].trim())
    .filter(name => name.indexOf('.') === -1)
    .map(n => `"${n}"`)

taurusKeywordsTmp = propertiesMD
    .split('###')
    .map(p => {
        const re = />([\s\S]+?)</g;
        return p.match(re)
    })
    .filter(n => !!n)
    .map(n => n[0].split(' ')[0].slice(1))
    .map(n => `"${n}"`)

taurusKeywords = propertiesMD
    .split('\n')
    .filter(l => l.indexOf('|') !== -1)
    .filter(l => l.indexOf('---') === -1)
    .map(l => l.split('|')[1].trim())
    .filter(l => {
        const re = /.*[\u4e00-\u9fa5]+.*/;
        return !re.test(l)
    })
    .map(n => `"${n}"`)

tarusPropertyKeywordsTmp = functionsMD
    .split('####')
    .filter(f => f.split('>')[0].indexOf('###') === -1)
    .map(f => f.split('>')[0].trim())
    .filter(name => name.indexOf('.') !== -1)
    .map(n => `"${n}"`)
    
tarusPropertyKeywords = tarusPropertyKeywordsTmp.concat(taurusKeywords)
taurusKeywords = tarusPropertyKeywords.concat(taurusKeywordsTmp)


fs.writeFile('./taurusDefKeywords.js', '', () => {})
fs.writeFile('./taurusKeywords.js', '', () => {})
fs.writeFile('./tarusPropertyKeywords.js', '', () => {})

fs.appendFile('./taurusDefKeywords.js', taurusDefKeywords.join(',\n'), () => {})
fs.appendFile('./taurusKeywords.js', taurusKeywords.join(',\n'), () => {})
fs.appendFile('./tarusPropertyKeywords.js', tarusPropertyKeywords.join(',\n'), () => {})


