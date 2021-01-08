import minimist from 'minimist';

const argv = minimist(process.argv.slice(2))

const { accountId, ticker, side, offset, volume, steps, triggerTime, finishTime } = argv

console.log(process.argv)
console.log(argv)
console.log(accountId, ticker, side, offset, volume, steps, triggerTime, finishTime, '---')