import fse from 'fs-extra';
import csv from 'fast-csv';
import { deepClone } from '__gUtils/busiUtils';

const path = require("path");

declare global {
    interface Window { 
        fileId: number;
        ELEC_WIN_MAP: any;
    }
}

export {}

interface FileIds {
    file: any[];
    folder: any[];
}

interface FileData {
    id: number;
    parentId: number;
    isDir: boolean;
    name: string;
    ext: string;
    filePath: string;
    children: FileIds;
    stats: object;
    root: boolean;
    open: boolean;
}

interface FileTreeByPath {
    ids: FileIds;
    fileTree: any;
}

interface FileInputData extends FileData {
    [propName: string]: any;
}


export const listDir = (filePath: string): Promise<void | string[]> => {
        fse.ensureDirSync(filePath)
        return fse.readdir(filePath).catch(err => console.error(err))
}

export const listDirSync = (filePath: string): void | string[] => {
    fse.ensureDirSync(filePath)
    return fse.readdirSync(filePath)
}

export const getTreeByFilePath = (strategy: FileData, fileTree: any): Promise<FileTreeByPath> => {
    fileTree = fileTree || {};
    let strategyPath: string = strategy.filePath;
    strategyPath = path.normalize(strategyPath)
    const filePath: string = path.resolve(strategyPath);
    let dirList: any[] = [];
    let fileList: any[] = [];
    let ids: FileIds = { file: [], folder: [] };
    const ignoreList = ['.git', '.DS_Store']
    return new Promise((resolve, reject) => {
        listDir(filePath).then(files => {
            (files || []).forEach((file: any) => {
                if(ignoreList.includes(file)) return;
                const fileDir: string = path.join(filePath, file);
                const stats: any = fse.statSync(fileDir)
                if(stats){
                    const isDir: boolean = stats.isDirectory();
                    const fileInfo: FileData = buildFileObj({
                            id: 0,
                            parentId: strategy.id,
                            isDir,
                            name: file,
                            ext: path.extname(fileDir).slice(path.extname(fileDir).indexOf('.') + 1),
                            filePath: fileDir,
                            children: {
                                folder: [],
                                file: []
                            },
                            stats,
                            root: false,
                            open: false
                    })
                    //保证顺序
                    if(isDir){
                        dirList.push(fileInfo)
                    }else{
                        fileList.push(fileInfo)
                    }
                }
            })
            dirList.forEach((dir: FileData): void => {
                const id: number = window.fileId++
                ids.folder.push(id);
                fileTree[id] = {
                    ...dir,
                    id
                }
            })
            fileList.forEach((file: FileData): void => {
                const id: number = window.fileId++
                ids.file.push(id);
                fileTree[id] = {
                    ...file,
                    id
                }
            })
            resolve({ ids, fileTree })
        })
        // .catch(err => reject(err))
    })    
}

//清空children
export const clearChildrenByFileId = (fileTree: any, fileId: number): any => {
    const target: any = fileTree[fileId]
    const children: any = target.children;
    const files: any = children['file'] || [];
    const folders: any = children['folders'] || [];

    [...files, ...folders].forEach(id => {
        fileTree[id] = null
        delete fileTree[id]
    }) 

    target.children = {
        file: [],
        folder: []
    }
    return fileTree
}


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
        parentId
    }
}

//打开文件树
/**
 * @param  {object} folder
 * @param  {object} oldFileTree
 * @param  {boolean} openStatus
 * @param  {boolean} force 强制清空
 */
export const openFolder = (store: any, folder: FileData, oldFileTree: any, openStatus?: boolean, force?: boolean): Promise<{}> => {
    oldFileTree = deepClone(oldFileTree);
    //清空
    if(force) oldFileTree = clearChildrenByFileId(oldFileTree, folder.id);
    return new Promise(async (resolve) => {
        if(openStatus === undefined){
            openStatus = !folder.open
        }
        if(openStatus){
            const { ids, fileTree }: FileTreeByPath = await getTreeByFilePath(folder, oldFileTree)    
            store.dispatch('setFileTree', fileTree)
            store.dispatch('setFileNode', {id: folder.id, attr: 'children', val: ids})
            resolve(fileTree)
        }else{
            resolve({})
        }
        //打开
        store.dispatch('setFileNode', {id: folder.id, attr: 'open', val: openStatus})
    })
    
}

//添加文件
export const addFileSync = (parentDir = "", filename: string, type: string): void => {
    let targetPath: string;
    if(!parentDir) targetPath = filename;
    else targetPath = path.join(parentDir, filename)
    targetPath = path.normalize(targetPath);
    
    if(type === 'folder'){
        fse.ensureDirSync(targetPath)
    }else{
        fse.ensureFileSync(targetPath)
    }
}

//更改文件名
export const editFileFolderName = (oldPath: string, newPath: string): Promise<void> => {
    oldPath = path.normalize(oldPath)
    newPath = path.normalize(newPath)
    return fse.rename(oldPath, newPath)
}


//删除文件或
export const removeFileFolder = (targetPath: string): Promise<void> => {
    targetPath = path.normalize(targetPath)
    return new Promise(resolve => {
        if(!fse.existsSync(targetPath)) {
            resolve()
            return
        } 
        fse.removeSync(targetPath)
        resolve()
    }) 
}

export const removeFilesInFolder = (targetDir: string) => {
    targetDir = path.normalize(targetDir)
    if(!fse.existsSync(targetDir)) throw new Error(`${targetDir} not existed!`)
    const promises = fse.readdirSync(targetDir).map((file: any) => {
        const filePath = path.join(targetDir, file)
        return removeFileFolder(filePath)
    });

    return Promise.all(promises)
}

//获取文件内容
export const getCodeText = (targetPath: string): Promise<string> => {
    if(!targetPath) throw new Error('文件路径不存在！')
    targetPath = path.normalize(targetPath)
    return new Promise((resolve, reject): void => {
        fse.readFile(targetPath, 'utf-8', (err: Error, data: any) => {
            if(err){
                console.error(err)
                reject(err)
                return;
            }
            resolve(data.toString())
        })
    })
}

//写入文件
export const writeFile = (filePath: string, data: string): Promise<void> => {
    filePath = path.normalize(filePath)
    if(data === undefined) throw new Error('input data is undefined!')
    return fse.outputFile(filePath, data)
}

export const writeCSV = (filePath: string, data: any[]): Promise<void> => {
    filePath = path.normalize(filePath)
    return new Promise((resolve) => {
        csv.writeToPath(filePath, data, {
            headers: true,
        }).on("finish", function(){
            resolve()
        })
    })
    
}

export const removeJournal = (targetFolder: string) => {
            
    function iterator (folder: string) {
        const items = listDirSync(folder)

        if (!items) return;
        
        const folders = items.filter((f: string) => {
            const stat = fse.statSync(path.join(folder, f))

            if (stat.isDirectory()) return true;
            return false;
        })

        const files = items.filter((f: string) => {
            const stat = fse.statSync(path.join(folder, f))

            if (stat.isFile()) return true;
            return false;
        })                

        files.forEach((f: string) => {
            if (f.includes('.journal')) {
                fse.removeSync(path.join(folder, f))
            }
        })

        folders.forEach((f: string) => {
            iterator(path.join(folder, f))
        })                
    }  

    iterator(targetFolder)

    return Promise.resolve(true)
}