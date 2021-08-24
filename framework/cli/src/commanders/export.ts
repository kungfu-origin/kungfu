import { statSync, ensureDirSync } from "fs-extra";
import moment from "moment";
import { getKungfuDataByDateRange } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import { ensureLedgerData } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { writeCSV } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';

const os = require('os');
const path = require('path');
const inquirer = require( 'inquirer' );
const { PathPrompt } = require('@kungfu-trader/kungfu-cli/assets/static/js/inquirer-path/lib');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));
inquirer.registerPrompt('path', PathPrompt);


const inputExportDateAndPath = async () => {
    return await inquirer.prompt([
        {
            type: 'input',
            name: 'date',
            default: moment().format('YYYY-MM-DD'),
            message: "Input date (format YYYY-MM-DD)",
            validate: (value: string) => {
                const dateFormat =/^(\d{4})-(\d{2})-(\d{2})$/;

                if (dateFormat.test(value)) {
                    return true;
                } else {
                    return new Error(`${value} is not in YYYY-MM-DD format`);
                }
            }
        }, 
        {
            type: 'path',
            name: 'output_folder',
            message: "Input trading data output folder",
            default: os.homedir(),
            validate: (value: string) => {

                const stats = statSync(value);
                if (stats && stats.isDirectory()) {
                    return true;
                } else {
                    return new Error(`${value} is not a folder path`);
                }
            } 
            
        }
    ])
}

export const exportTradingDataPrompt = async () => {
    const { flatternOrderTrades } = require('@kungfu-trader/kungfu-uicc/io/kungfu/watcher');
    const { date, output_folder } = await inputExportDateAndPath();

    const outputPathResolved = path.join(output_folder, `kungfu-trading-data-${date}`);
    const ordersFilename = path.join(outputPathResolved, `orders-${date}`);
    const tradesFilename = path.join(outputPathResolved, `trades-${date}`);
    const orderStatFilename = path.join(outputPathResolved, `orderStats-${date}`);
    const posFilename = path.join(outputPathResolved, `pos-${date}`);

    console.log(
    `Start export
    ${ordersFilename}
    ${tradesFilename}
    ${orderStatFilename}
    ${posFilename}
Please waiting...` // has to be in this indent way
    )

    

    return getKungfuDataByDateRange(date)
        .then((kungfuData: any) => {
            
            if (!kungfuData) {
                throw new Error('Export failed');
            }
            
            const orders = ensureLedgerData(kungfuData.Order, 'update_time'); 
            const ordersResolved = flatternOrderTrades(orders);
            const trades = ensureLedgerData(kungfuData.Trade, 'trade_time');
            const tradesResolved = flatternOrderTrades(trades);
            const orderStat = ensureLedgerData(kungfuData.OrderStat, 'insert_time');
            const positions = ensureLedgerData(kungfuData.Position);
            
            ensureDirSync(outputPathResolved)

            return Promise.all([
                writeCSV(ordersFilename, ordersResolved),
                writeCSV(tradesFilename, tradesResolved),
                writeCSV(orderStatFilename, orderStat),
                writeCSV(posFilename, positions),
            ])
        })
        .then(() => {
            return outputPathResolved
        })
}