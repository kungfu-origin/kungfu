import { MonitorDashboard } from 'src/components/monitor';
import { globalState } from './globalState';

export const initBusEvent = (dashboard: MonitorDashboard) => {
  const { message, loader } = dashboard.boards;
  globalState.GLOBAL_BUS.subscribe((data) => {
    if (message && loader) {
      const { tag, name, status } = data;
      if (tag === 'processStatus') {
        switch (name) {
          case 'system':
            loader.load(`Restart system, Please wait...`);
            break;
          case 'archive':
            loader.stop();
            if (status === 'online') {
              message.log(`Archive success`, 2, (err) => {
                if (err) {
                  console.error(err);
                }
              });
            }
            break;
          case 'master':
            if (status === 'online') {
              message.log(`Master success`, 2, (err) => {
                if (err) {
                  console.error(err);
                }
              });
            }
            break;
          case 'ledger':
            if (status === 'online') {
              message.log(`Ledger success`, 2, (err) => {
                if (err) {
                  console.error(err);
                }
              });
            }
            break;
        }
      }
    }
  });
};
