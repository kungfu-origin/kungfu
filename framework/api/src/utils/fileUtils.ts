import path from 'path';
import fse from 'fs-extra';
import * as csv from '@fast-csv/format';
import { Row } from '@fast-csv/format';

interface FileInputData extends Code.FileData {
  [propName: string]: any;
}
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

export const listDir = (filePath: string): Promise<void | string[]> => {
  fse.ensureDirSync(filePath);
  return fse.readdir(filePath).catch((err) => console.error(err));
};


//写入文件
export const writeFile = (filePath: string, data: string): Promise<void> => {
  filePath = path.normalize(filePath);
  if (data === undefined) throw new Error('input data is undefined!');
  return fse.outputFile(filePath, data);
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

//建立fileObj
export const buildFileObj = (fileData: FileInputData): Code.FileData => {
  const {
    id,
    isDir,
    name,
    ext,
    filePath,
    children,
    stats,
    root,
    open,
    parentId,
  } = fileData;
  return {
    id,
    isDir,
    name,
    ext,
    filePath,
    children,
    stats,
    root,
    open,
    parentId,
  };
};



//更改文件名
export const editFileFolderName = (
  oldPath: string,
  newPath: string,
): Promise<void> => {
  oldPath = path.normalize(oldPath);
  newPath = path.normalize(newPath);
  return fse.rename(oldPath, newPath);
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

      if (stat.isDirectory()) return true;
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
