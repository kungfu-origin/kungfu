import '../assets/methods/setExportEvn';
import inquirer from 'inquirer';
import dayjs from 'dayjs';
import os from 'os';
import path from 'path';
import fse from 'fs-extra';
import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import {
  dealTradingDataItem,
  getKungfuHistoryData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { writeCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { Row } from '@fast-csv/format';

const inputExportDateAndPath = async () => {
  return await inquirer.prompt([
    {
      type: 'input',
      name: 'date',
      default: dayjs().format('YYYY-MM-DD'),
      message: 'Input date (format YYYY-MM-DD)',
      validate: (value: string) => {
        const dateFormat = /^(\d{4})-(\d{2})-(\d{2})$/;

        if (dateFormat.test(value)) {
          return true;
        } else {
          throw new Error(`${value} is not in YYYY-MM-DD format`);
        }
      },
    },
    {
      type: 'path',
      name: 'output_folder',
      message: 'Input trading data output folder',
      default: os.homedir(),
      validate: (value: string) => {
        const stats = fse.statSync(value);
        if (stats && stats.isDirectory()) {
          return true;
        } else {
          throw new Error(`${value} is not a folder path`);
        }
      },
    },
  ]);
};

const dealTradingDataItemResolved = (item: KungfuApi.TradingDataTypes): Row => {
  return dealTradingDataItem(item, watcher) as Row;
};

export const exportTradingData = async (date, output_folder) => {
  const { tradingData } = await getKungfuHistoryData(
    watcher,
    date,
    HistoryDateEnum.naturalDate,
    'all',
  );

  const orders = tradingData.Order.sort('update_time');
  const trades = tradingData.Trade.sort('trade_time');
  const orderStat = tradingData.OrderStat.sort('insert_time');
  const positions = tradingData.Trade.list();

  const ordersFilename = path.join(output_folder, `orders-${date}}`);
  const tradesFilename = path.join(output_folder, `trades-${date}`);
  const orderStatFilename = path.join(output_folder, `orderStats-${date}`);
  const posFilename = path.join(output_folder, `pos-${date}`);

  return Promise.all([
    writeCSV(ordersFilename, orders, dealTradingDataItemResolved),
    writeCSV(tradesFilename, trades, dealTradingDataItemResolved),
    writeCSV(orderStatFilename, orderStat, dealTradingDataItemResolved),
    writeCSV(posFilename, positions, dealTradingDataItemResolved),
  ]);
};

export const exportTradingDataPrompt = async () => {
  const { date, output_folder } = await inputExportDateAndPath();
  await exportTradingData(date, output_folder);
  return output_folder;
};
