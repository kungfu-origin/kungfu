import inquirer from 'inquirer';
import dayjs from 'dayjs';
import os from 'os';
import path from 'path';
import fse from 'fs-extra';
import { exportTradingData } from '../assets/actions/tradingData';

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
      default: path.resolve(os.homedir()),
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

export const exportTradingDataPrompt = async () => {
  const { date, output_folder } = await inputExportDateAndPath();
  await exportTradingData(date, output_folder);
  return output_folder;
};
