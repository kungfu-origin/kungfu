import path from 'path';
import fse from 'fs-extra';

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
