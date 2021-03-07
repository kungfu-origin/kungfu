
const minimist = require('minimist')
const fse = require('fs-extra')
const csv = require('fast-csv');
const moment = require('moment');
const path = require('path');

const argv = minimist(process.argv.slice(2))

const { days, dataPath } = argv;

const targetFilenamesFromDays = buildDaysDate(days).map(name => path.resolve(dataPath, `${name}.csv`));

const promises = targetFilenamesFromDays
    .filter(filename => {
        return fse.existsSync(filename)
    })
    .map(filename => getMarketDatanByFile(filename));

Promise.all(promises)
    .then(quotesList => {
        const avgVolumeData = calcAvgVolume(quotesList)

        console.log('Calculate Results: ', avgVolumeData)
        
        process.send({
            type: 'process:msg',
            data: {
                type: 'CALC_AVG_VOLUME',
                body: {
                    days,
                    data: avgVolumeData 
                }
            }
        })

    })


function getMarketDatanByFile (filename) {
    let table = [];
    return new Promise((resolve, reject) => {
        fse.createReadStream(filename)
            .pipe(csv.parse({ headers: true }))
            .on('error', error => reject(error))
            .on('data', row => {
                table.push(row)
            })
            .on('end', rowCount => resolve(table));
    })
}

function buildDaysDate (daysLen) {
    daysLen = daysLen || 7;
    let i = 0, daysDate = [];
    for (i; i < daysLen; i++) {
        daysDate.push(moment().subtract((i + 1), 'd').format('YYYY-MM-DD'))
    }
    return daysDate
}

function buildVolumeDataFromArrayById (targetList, ids) {
    ids = ids ? ids.split(',').map(key => key.trim()) : [];
    
    let resultData = {}
    targetList.forEach(target => {
        let fullKeyList = [];
        ids.forEach(id => {
            fullKeyList.push(target[id] || '')
        })
        resultData[fullKeyList.join('_')] = +target.volume || 0
    })

    return resultData
}

function calcAvgVolume (quotesList) {
    let avgVolumeData = {}
    quotesList.forEach(quotes => {
        const quotesVolumeData = buildVolumeDataFromArrayById(quotes, 'instrument_id, exchange_id');

        Object.keys(quotesVolumeData || {})
            .forEach(key => {
                if (!avgVolumeData[key]) {
                    avgVolumeData[key] = {
                        val: 0,
                        count: 0
                    }
                }
                
                if (+quotesVolumeData[key] !== 0) {
                    avgVolumeData[key].val = avgVolumeData[key].val + quotesVolumeData[key]
                    avgVolumeData[key].count = avgVolumeData[key].count + 1
                }
            })
    })

    Object.keys(avgVolumeData || {})
        .forEach(key => {
            const volumeData = avgVolumeData[key] || {}
            const { val, count } = volumeData;
            if (+count) {
                avgVolumeData[key] = Number(val / count).toFixed(0)
            } else {
                avgVolumeData[key] = 0
            }
        })

    return avgVolumeData
} 