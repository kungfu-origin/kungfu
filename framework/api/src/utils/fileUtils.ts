import path from 'path';
import fse from 'fs-extra';
import * as csv from '@fast-csv/format';
import { Row } from '@fast-csv/format';

//添加文件
export const addFileSync = (
  parentDir = '',
  filename: string,
  type: 'folder' | 'file',
  content?: string,
): void => {
  let targetPath: string;
  if (!parentDir) targetPath = filename;
  else targetPath = path.join(parentDir, filename);
  targetPath = path.normalize(targetPath);

  if (type === 'folder') {
    fse.ensureDirSync(targetPath);
  } else {
    fse.ensureFileSync(targetPath);
    if (content) {
      fse.writeFileSync(targetPath, content);
    }
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


//获取文件内容
export const getCodeText = (targetPath: string): Promise<string> => {
  if (!targetPath) throw new Error('文件路径不存在！');
  targetPath = path.normalize(targetPath);
  return new Promise((resolve, reject): void => {
    fse.readFile(targetPath, 'utf-8', (err: Error, data: any) => {
      if (err) {
        console.error(err);
        reject(err);
        return;
      }
      resolve(data.toString());
    });
  });
};

//写入文件
export const writeFile = (filePath: string, data: string): Promise<void> => {
  filePath = path.normalize(filePath);
  if (data === undefined) throw new Error('input data is undefined!');
  return fse.outputFile(filePath, data);
};
