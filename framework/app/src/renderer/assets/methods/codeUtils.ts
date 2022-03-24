import fse from 'fs-extra';
import path from 'path';
import {
  listDir,
  buildFileObj,
} from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { Stats } from 'fs-extra';

// import { deepClone } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

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
    // .catch(err => reject(err))
  });
};

// //打开文件树
// export const openFolder = (
//   store: any,
//   folder: Code.FileData,
//   oldFileTree: Code.IFileTree,
//   openStatus?: boolean,
//   force?: boolean,
// ) => {
//   oldFileTree = deepClone(oldFileTree);
//   //清空
//   if (force) oldFileTree = clearChildrenByFileId(oldFileTree, folder.id);
//   return new Promise(async (resolve) => {
//     if (openStatus === undefined) {
//       openStatus = !folder.open;
//     }
//     if (openStatus) {
//       const { ids, fileTree }: Code.FileTreeByPath = await getTreeByFilePath(
//         folder,
//         oldFileTree,
//       );
//       store.dispatch('setFileTree', fileTree);
//       store.dispatch('setFileNode', {
//         id: folder.id,
//         attr: 'children',
//         val: ids,
//       });
//       resolve(fileTree);
//     } else {
//       resolve({});
//     }
//     //打开
//     store.dispatch('setFileNode', {
//       id: folder.id,
//       attr: 'open',
//       val: openStatus,
//     });
//   });
// };

// //清空children
// export const clearChildrenByFileId = (fileTree: any, fileId: number): any => {
//   const target: any = fileTree[fileId];
//   const children: any = target.children;
//   const files: any = children['file'] || [];
//   const folders: any = children['folders'] || [];

//   [...files, ...folders].forEach((id) => {
//     fileTree[id] = null;
//     delete fileTree[id];
//   });

//   target.children = {
//     file: [],
//     folder: [],
//   };
//   return fileTree;
// };
