import { ipcRenderer } from 'electron';
import {
  getStrategyById,
  updateStrategyPath,
} from '@kungfu-trader/kungfu-js-api/kungfu/strategy';
import { BrowserWindow } from '@electron/remote';

export function bindIPCListener(store, app) {
  ipcRenderer.removeAllListeners('ipc-emit-strategyById');
  ipcRenderer.on('ipc-emit-strategyById', (event, { childWinId, params }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    const { strategyId } = params;
    return getStrategyById(strategyId).then((strategies) => {
      if (childWin) {
        childWin.webContents.send('ipc-res-strategyById', strategies);
      }
    });
  });

  ipcRenderer.removeAllListeners('ipc-emit-updateStrategyPath');
  ipcRenderer.on(
    'ipc-emit-updateStrategyPath',
    (event, { childWinId, params }) => {
      console.log(app);

      const childWin = BrowserWindow.fromId(childWinId);
      const { strategyId, strategyPath } = params;
      return updateStrategyPath(strategyId, strategyPath).then(() => {
        if (childWin) {
          app &&
            app.emit('confirm', {
              formState: {
                strategy_id: strategyId,
                strategy_path: strategyPath,
              },
              strategyId,
              changeType: 'update',
            });
          childWin.webContents.send('ipc-res-updateStrategyPath');
        }
      });
    },
  );

  ipcRenderer.removeAllListeners('ipc-emit-strategyList');
  ipcRenderer.on('ipc-emit-strategyList', (event, { childWinId }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    return new Promise(() => {
      if (childWin) {
        childWin.webContents.send('ipc-res-strategyList', store.strategyList);
      }
    });
  });
}
