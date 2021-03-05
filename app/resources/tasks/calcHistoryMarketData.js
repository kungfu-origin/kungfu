
const minimist = require('minimist')
const fse = require('fse')
const fastCsv = require('fast-csv');

const argv = minimist(process.argv.slice(2))

console.log(process.env)
console.log(argv)