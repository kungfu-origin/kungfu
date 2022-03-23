import path from 'path';
import fse from 'fs-extra';
import * as csv from '@fast-csv/format';
import { Row } from '@fast-csv/format';
import { deepClone } from './busiUtils';

interface FileInputData extends FileData {
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

export const getTreeByFilePath = (
  strategy: FileData,
  fileTree: any,
): Promise<FileTreeByPath> => {
  fileTree = fileTree || {};
  let strategyPath: string = strategy.filePath;
  strategyPath = path.normalize(strategyPath);
  const filePath: string = path.resolve(strategyPath);
  let dirList: any[] = [];
  let fileList: any[] = [];
  let ids: FileIds = { file: [], folder: [] };
  const ignoreList = ['.git', '.DS_Store'];
  return new Promise((resolve, reject) => {
    listDir(filePath).then((files) => {
      (files || []).forEach((file: string) => {
        if (ignoreList.includes(file)) return;
        const fileDir: string = path.join(filePath, file);
        const stats: any = fse.statSync(fileDir);
        if (stats) {
          const isDir: boolean = stats.isDirectory();
          const fileInfo: FileData = buildFileObj({
            id: 0,
            parentId: strategy.id,
            isDir,
            name: file,
            ext: path
              .extname(fileDir)
              .slice(path.extname(fileDir).indexOf('.') + 1),
            filePath: fileDir,
            children: {
              folder: [],
              file: [],
            },
            stats,
            root: false,
            open: false,
          });
          //保证顺序
          if (isDir) {
            dirList.push(fileInfo);
          } else {
            fileList.push(fileInfo);
          }
        }
      });
      dirList.forEach((dir: FileData): void => {
        const id: number = window.fileId++;
        ids.folder.push(id);
        fileTree[id] = {
          ...dir,
          id,
        };
      });
      fileList.forEach((file: FileData): void => {
        const id: number = window.fileId++;
        ids.file.push(id);
        fileTree[id] = {
          ...file,
          id,
        };
      });
      resolve({ ids, fileTree });
    });
    // .catch(err => reject(err))
  });
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
export const buildFileObj = (fileData: FileInputData): FileData => {
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

//打开文件树
/**
 * @param  {object} folder
 * @param  {object} oldFileTree
 * @param  {boolean} openStatus
 * @param  {boolean} force 强制清空
 */
export const openFolder = (
  store: any,
  folder: FileData,
  oldFileTree: any,
  openStatus?: boolean,
  force?: boolean,
): Promise<{}> => {
  oldFileTree = deepClone(oldFileTree);
  //清空
  if (force) oldFileTree = clearChildrenByFileId(oldFileTree, folder.id);
  return new Promise(async (resolve) => {
    if (openStatus === undefined) {
      openStatus = !folder.open;
    }
    if (openStatus) {
      const { ids, fileTree }: FileTreeByPath = await getTreeByFilePath(
        folder,
        oldFileTree,
      );
      store.dispatch('setFileTree', fileTree);
      store.dispatch('setFileNode', {
        id: folder.id,
        attr: 'children',
        val: ids,
      });
      resolve(fileTree);
    } else {
      resolve({});
    }
    //打开
    store.dispatch('setFileNode', {
      id: folder.id,
      attr: 'open',
      val: openStatus,
    });
  });
};

//清空children
export const clearChildrenByFileId = (fileTree: any, fileId: number): any => {
  const target: any = fileTree[fileId];
  const children: any = target.children;
  const files: any = children['file'] || [];
  const folders: any = children['folders'] || [];

  [...files, ...folders].forEach((id) => {
    fileTree[id] = null;
    delete fileTree[id];
  });

  target.children = {
    file: [],
    folder: [],
  };
  return fileTree;
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