import {
  killExtra,
  killKfc,
  pm2Kill,
  pm2KillGodDaemon,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import 'console-success';
import { deleteNNFiles } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

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
