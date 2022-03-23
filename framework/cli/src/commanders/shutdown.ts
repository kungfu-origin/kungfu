import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  killExtra,
  killKfc,
  pm2Kill,
  pm2KillGodDaemon,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import glob from 'glob';
import path from 'path';
import fse from 'fs-extra';
import 'console-success';

export function deleteNNFiles(rootPathName = KF_HOME) {
  return new Promise((resolve, reject) => {
    glob(
      '**/*.nn',
      {
        cwd: rootPathName,
      },
      (err: Error, files: string[]) => {
        if (err) {
          reject(err);
        }

        files.forEach((file: string) => {
          console.log('Deleting ', path.join(rootPathName, file));
          fse.unlinkSync(path.join(rootPathName, file));
        });

        resolve(true);
      },
    );
  });
}

export const shutdown = async () => {
  try {
    console.log('Killing all Process ...');
    await pm2Kill();
    console.log('Killing pm2 ...');
    await pm2KillGodDaemon();
    console.log('Killing extra ...');
    await killKfc();
    await killExtra();
    console.log('Deleting socket files ...');
    await deleteNNFiles();

    console.success(`Shutdown kungfu`);
    process.exit(0);
  } catch (err) {
    console.error(err);
    process.exit(1);
  }
};
