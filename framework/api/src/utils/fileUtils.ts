import path from 'path';
import fse from 'fs-extra';
import * as csv from '@fast-csv/format';
import { Row } from '@fast-csv/format';

//添加文件
export const addFileSync = (
  parentDir = '',
  filename: string,
  type: string,
): void => {
  let targetPath: string;
  if (!parentDir) targetPath = filename;
  else targetPath = path.join(parentDir, filename);
  targetPath = path.normalize(targetPath);

  if (type === 'folder') {
    fse.ensureDirSync(targetPath);
  } else {
    fse.ensureFileSync(targetPath);
  }
};

export const writeCSV = (
  filePath: string,
  data: KungfuApi.TradingDataTypes[],
  transform = (row: KungfuApi.TradingDataTypes) => row as Row,
): Promise<void> => {
  filePath = path.normalize(filePath);
  return new Promise((resolve, reject) => {
    csv
      .writeToPath(filePath, data, {
        headers: true,
        transform: transform,
      })
      .on('finish', function () {
        resolve();
      })
      .on('error', (err) => {
        reject(err);
      });
  });
};
