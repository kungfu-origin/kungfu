import { remote, ipcRenderer } from 'electron';

const currentWin = remote.getCurrentWindow();
const paWin = currentWin.getParentWindow();

ipcRenderer.setMaxListeners(10);

export const ipcEmitDataByName = (
  name: string,
  postData?: any,
  interval?: number,
) => {
  interval = interval || 5000;

  if (!paWin) {
    return Promise.reject(new Error('paWin is null!'));
  }

  return new Promise((resolve, reject) => {
    paWin.webContents.send(`ipc-emit-${name}`, {
      childWinId: currentWin.id,
      params: Object.freeze(postData || {}),
    });

    let timer = setTimeout(() => {
      reject(new Error(`ipc-emit-${name} timeout`));
      ipcRenderer.removeAllListeners(`ipc-res-${name}`);
      clearTimeout(timer);
    }, interval);

    ipcRenderer.on(`ipc-res-${name}`, (event: any, data: any) => {
      resolve({ event, data });
      ipcRenderer.removeAllListeners(`ipc-res-${name}`);
      clearTimeout(timer);
    });
  });
};