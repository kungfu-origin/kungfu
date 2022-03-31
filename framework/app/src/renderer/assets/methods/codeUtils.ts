import fse from 'fs-extra';
import path from 'path';
import { listDir } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { Stats } from 'fs-extra';
import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useCodeStore } from '../../pages/code/store/codeStore';

export const getTreeByFilePath = (
  strategy: Code.FileData,
  fileTree: Code.IFileTree,
): Promise<Code.FileTreeByPath> => {
  fileTree = fileTree || {};
  let strategyPath: string = strategy.filePath;
  strategyPath = path.normalize(strategyPath);
  const filePath: string = path.resolve(strategyPath);
  const dirList: Code.FileData[] = [];
  const fileList: Code.FileData[] = [];
  const ids: Code.FileIds = { file: [], folder: [] };
  const ignoreList = ['.git', '.DS_Store'];
  return new Promise((resolve) => {
    listDir(filePath).then((files) => {
      (files || []).forEach((file: string) => {
        if (ignoreList.includes(file)) return;
        const fileDir: string = path.join(filePath, file);
        const stats: Stats = fse.statSync(fileDir);
        if (stats) {
          const isDir: boolean = stats.isDirectory();
          const fileInfo: Code.FileData = buildFileObj({
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
            fileId: 1,
          });
          //保证顺序
          if (isDir) {
            dirList.push(fileInfo);
          } else {
            fileList.push(fileInfo);
          }
        }
      });

      dirList.forEach((dir: Code.FileData): void => {
        const id: number = window.fileId++;
        ids.folder.push(id);
        fileTree[id] = {
          ...dir,
          id,
        };
      });
      fileList.forEach((file: Code.FileData): void => {
        const id: number = window.fileId++;
        ids.file.push(id);
        fileTree[id] = {
          ...file,
          id,
        };
      });
      resolve({ ids, fileTree });
    });
  });
};

//建立fileObj
export const buildFileObj = (fileData: Code.FileData): Code.FileData => {
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
    fileId,
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
    fileId,
    parentId,
  };
};

// //打开文件树
export const openFolder = async (
  folder: Code.FileData,
  oldFileTree: Code.IFileTree,
  openStatus?: boolean,
  force?: boolean,
): Promise<Code.IFileTree> => {
  const store = useCodeStore();
  oldFileTree = deepClone(oldFileTree);
  //清空
  if (force) {
    oldFileTree = clearChildrenByFileId(oldFileTree, folder.id);
  }
  if (openStatus === undefined) {
    openStatus = !folder.open;
  }
  if (openStatus) {
    const { ids, fileTree }: Code.FileTreeByPath = await getTreeByFilePath(
      folder,
      oldFileTree,
    );
    const newFileTree: Code.IFileTree = reSetParentFileId(fileTree, folder);
    // folder.fileId = (folder.fileId || 0) + 1;
    // fileTree[0].fileId = (fileTree[0].fileId || 0) + 1;
    store.setFileTree(newFileTree);
    store.setFileNode({
      id: folder.id,
      attr: 'children',
      val: ids,
    });
    //打开
    store.setFileNode({
      id: folder.id,
      attr: 'open',
      val: openStatus,
    });
    return newFileTree;
  } else {
    //关闭
    const fileClosed = reSetParentFileId(
      clearChildrenByFileId(oldFileTree, folder.id),
      folder,
    );
    // fileClosed[0].fileId = (fileClosed[0].fileId || 0) + 1;
    store.setFileTree(fileClosed);
    store.setFileNode({
      id: folder.id,
      attr: 'open',
      val: false,
    });
    return {};
  }
};

// //清空children
export const clearChildrenByFileId = (
  fileTree: Code.IFileTree,
  fileId: number,
): Code.IFileTree => {
  const target: Code.FileData = fileTree[fileId];
  const children: { file: Array<number>; folder: Array<number> } =
    target.children;
  const files: Array<number> = children['file'] || [];
  const folders: Array<number> = children['folders'] || [];
  const returnFile: Code.IFileTree = deepClone(fileTree);
  [...files, ...folders].forEach((id) => {
    delete returnFile[id];
  });

  returnFile[fileId].children = {
    file: [],
    folder: [],
  };
  return returnFile;
};

function reSetParentFileId(
  fileTree: Code.IFileTree,
  folder: Code.FileData,
): Code.IFileTree {
  const currentId: number = folder.parentId == 0 ? folder.id : folder.parentId;
  const fileId: number = (fileTree[currentId].fileId || 0) + 1;
  fileTree[currentId].fileId = fileId;
  return fileTree;
}
