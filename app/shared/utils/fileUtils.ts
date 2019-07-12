import { deepClone } from '__gUtils/busiUtils';
const path = require("path");
const fse = require('fs-extra');
const csv = require("fast-csv");

declare global {
    interface Window { 
        fileId: number;
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



export const getTreeByFilePath = (strategy: FileData, fileTree: any): Promise<FileTreeByPath> => {
    let strategyPath: string = strategy.filePath;
    strategyPath = path.normalize(strategyPath)
    const filePath: string = path.resolve(strategyPath);
    let dirList: any[] = [];
    let fileList: any[] = [];
    let ids: FileIds = { file: [], folder: [] };
    const ignoreList = ['.git', '.DS_Store']
    return new Promise((resolve, reject) => {
        fse.readdir(filePath, (err: Error, files: any[]) => {
            if(err) {
                console.error(err)
                reject(err)
            }else {
                files.forEach((file: any) => {
                    if(ignoreList.indexOf(file) !== -1) return;
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
            }
        })
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
export const addFile = (parentDir = "", filename: string, type: string): void => {
    let targetPath: string;
    if(!parentDir) targetPath = filename;
    else targetPath = path.join(parentDir, filename)
    targetPath = path.normalize(targetPath)
    if(type === 'folder'){
        fse.ensureDirSync(targetPath)
    }else{
        fse.ensureFileSync(targetPath)
    }
}

//更改文件名
export const editFileFolderName = (oldPath: string, newPath: string): Promise<{}> => {
    oldPath = path.normalize(oldPath)
    newPath = path.normalize(newPath)
    return new Promise((resolve, reject) => {
        fse.rename(oldPath, newPath, (err: Error) => {
            if(err){
                console.error(err)
                reject(err)
                return
            }
            resolve({})
        })
    })
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

//获取文件内容
export const getCodeText = (targetPath: string): Promise<string> => {
    if(!targetPath) throw new Error('文件路径不存在！')
    targetPath = path.normalize(targetPath)
    return new Promise((resolve, reject): void => {
        fse.readFile(targetPath, (err: Error, data: any) => {
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
    return new Promise((resolve, reject) => {
        if(data === undefined) reject(new Error('input data is undefined!'))
        fse.outputFile(filePath, data, (err: Error): void => {
            if(err){
                console.error(err)
                reject(err)
                return;
            }
            resolve()
        })
    })
}

export const writeCSV = (filePath: string, data: string): Promise<void> => {
    filePath = path.normalize(filePath)
    return new Promise((resolve) => {
        csv.writeToPath(filePath, data, {
            headers: true,
        }).on("finish", function(){
            resolve()
        })
    })
    
}

//清空文件内容
export const clearFileContent = (filePath: string): Promise<any> => {
    filePath = path.normalize(filePath)
    return new Promise((resolve, reject) => {
        fse.outputFile(filePath, '', (err: Error, data: any): void => {
            if(err){
                reject(err);
                return;
            }
            resolve(data)
        })
    })
}

//打开查看文件
export const openReadFile = (logPath: string): void => {
    addFile('', logPath, 'file')
    const shell = require('electron').shell;
    shell.openItem(logPath)
}

export const existsSync = (filePath: string): boolean => {
    return fse.existsSync(filePath)
}

export const copySync = (fromPath: string, toPath: string): void => {
    fse.copySync(fromPath, toPath)
}