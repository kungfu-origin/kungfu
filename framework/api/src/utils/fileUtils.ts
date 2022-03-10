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

export const removeFileFolder = (targetPath: string): Promise<void> => {
  targetPath = path.normalize(targetPath);
  return new Promise((resolve) => {
    if (!fse.existsSync(targetPath)) {
      resolve();
      return;
    }
    fse.removeSync(targetPath);
    resolve();
  });
};

export const removeFilesInFolder = (targetDir: string) => {
  targetDir = path.normalize(targetDir);
  if (!fse.existsSync(targetDir)) throw new Error(`${targetDir} not existed!`);
  const promises = fse.readdirSync(targetDir).map((file: any) => {
    const filePath = path.join(targetDir, file);
    return removeFileFolder(filePath);
  });

  return Promise.all(promises);
};
