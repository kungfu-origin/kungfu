import { ipcRenderer, remote } from 'electron';
import {
  getStrategyById,
  updateStrategyPath,
} from '@kungfu-trader/kungfu-js-api/kungfu/store';
const { BrowserWindow } = remote;

export function bindIPCListener() {
  ipcRenderer.removeAllListeners('ipc-emit-strategyById');
  ipcRenderer.on('ipc-emit-strategyById', (event, { childWinId, params }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    const { strategyId } = params;
    return getStrategyById(strategyId).then((strategies) => {
      if (childWin) {
        childWin.webContents.send(
          'ipc-res-strategyById',
          Object.freeze(strategies),
        );
      }
    });
  });

  ipcRenderer.removeAllListeners('ipc-emit-updateStrategyPath');
  ipcRenderer.on(
    'ipc-emit-updateStrategyPath',
    (event, { childWinId, params }) => {
      const childWin = BrowserWindow.fromId(childWinId);
      const { strategyId, strategyPath } = params;
      return updateStrategyPath(strategyId, strategyPath).then(() => {
        if (childWin) {
          childWin.webContents.send('ipc-res-updateStrategyPath');
        }
      });
    },
  );

  ipcRenderer.removeAllListeners('ipc-emit-strategyList');
  ipcRenderer.on('ipc-emit-strategyList', (event, { childWinId }) => {
    const childWin = BrowserWindow.fromId(childWinId);
    return this.$store.dispatch('getStrategyList').then((strategyList) => {
      if (childWin) {
        childWin.webContents.send(
          'ipc-res-strategyList',
          Object.freeze(strategyList),
        );
      }
    });
  });
}
