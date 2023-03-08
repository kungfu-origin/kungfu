import path from 'path';
import fse from 'fs-extra';
import * as csv from 'fast-csv';
import { FormatterRow, ParserOptionsArgs } from 'fast-csv';
import findRoot from 'find-root';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { RootConfigJSON } from '../typings/global';
const { t } = VueI18n.global;

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

export const readCSV = <T>(
  filepath: string,
  headers: ParserOptionsArgs['headers'],
  options?: {
    transformer?: (row) => T;
    validator?: (row) => boolean;
  },
) => {
  filepath = path.normalize(filepath);
  return new Promise<{
    resRows: T[];
    errRows: Array<{ row: number; data: Array<string | number | boolean> }>;
  }>((resolve, reject) => {
    const resRows: T[] = [];
    const errRows: Array<{
      row: number;
      data: Array<string | number | boolean>;
    }> = [];

    let parsing = csv.parseFile(filepath, {
      headers: headers,
      skipLines: headers === true ? 0 : 1,
    });

    if (options?.validator) {
      parsing = parsing.validate(options.validator);
    }

    parsing
      .on('data', function (row) {
        resRows.push(options?.transformer ? options.transformer(row) : row);
      })
      .on('data-invalid', function (data, row) {
        errRows.push({
          data,
          row,
        });
      })
      .on('end', function () {
        resolve({ resRows, errRows });
      })
      .on('error', (err) => {
        reject(err);
      });
  });
};

export const writeCSV = (
  filePath: string,
  data: KungfuApi.TradingDataTypes[],
  transform = (row: KungfuApi.TradingDataTypes) => row as FormatterRow,
): Promise<void> => {
  filePath = path.normalize(filePath);
  return new Promise((resolve, reject) => {
    csv
      .writeToPath(filePath, data, {
        headers: true,
        transform,
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
export const getFileContent = (targetPath: string): Promise<string> => {
  if (!targetPath) throw new Error(t('文件路径不存在'));
  targetPath = path.normalize(targetPath);
  return new Promise((resolve, reject): void => {
    const file = fse.createReadStream(targetPath);
    const fileContextList: Array<Buffer> = [];
    file.on('data', (data) => {
      fileContextList.push(data as Buffer);
    });
    file.on('end', () => {
      resolve(Buffer.concat(fileContextList).toString());
    });
    file.on('error', (err) => {
      reject(err);
      return;
    });
  });
};

export const listDir = (filePath: string): Promise<void | string[]> => {
  fse.ensureDirSync(filePath);
  return fse.readdir(filePath).catch((err) => console.error(err));
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
  const promises = fse.readdirSync(targetDir).map((file: string) => {
    const filePath = path.join(targetDir, file);
    return removeFileFolder(filePath);
  });

  return Promise.all(promises);
};

export const listDirSync = (filePath: string): string[] => {
  fse.ensureDirSync(filePath);
  return fse.readdirSync(filePath);
};

export const removeTargetFilesInFolder = (
  targetFolder: string,
  includes: string[],
  filters: string[] = [],
): Promise<void> => {
  const iterator = (folder: string) => {
    const items = listDirSync(folder);

    if (!items) return;

    const folders = items.filter((f: string) => {
      const stat = fse.statSync(path.join(folder, f));

      if (stat.isDirectory() && !filters.includes(f)) return true;
      return false;
    });

    const files = items.filter((f: string) => {
      const stat = fse.statSync(path.join(folder, f));

      if (stat.isFile()) return true;
      return false;
    });

    files.forEach((f: string) => {
      includes.forEach((n: string) => {
        if (f.includes(n) && !filters.includes(f)) {
          fse.removeSync(path.join(folder, f));
        }
      });
    });

    folders.forEach((f: string) => {
      iterator(path.join(folder, f));
    });
  };

  iterator(targetFolder);

  return Promise.resolve();
};

export const findPackageRoot = () => {
  const cwd = process.cwd().toString();
  const dirname = path.resolve(__dirname);
  let searchPath = '';
  if (process.env.NODE_ENV === 'production') {
    searchPath = dirname;
  } else {
    searchPath = cwd;
  }
  if (searchPath.includes('node_modules')) {
    return findRoot(path.resolve(searchPath.split('node_modules')[0]));
  }
  return findRoot(path.resolve(searchPath));
};

export const readRootPackageJsonSync = (): RootConfigJSON => {
  const rootDir = findPackageRoot();
  const packageJsonPath = path.join(rootDir, 'package.json');
  if (fse.existsSync(packageJsonPath)) {
    try {
      return fse.readJSONSync(packageJsonPath);
    } catch (err) {
      console.error(err);
      return {};
    }
  }

  return {};
};
