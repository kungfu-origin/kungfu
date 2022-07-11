import { MonitorDashboard } from 'src/components/monitor';
import { globalState } from './globalState';

export const initBusEvent = (dashboard: MonitorDashboard) => {
  const boards = dashboard.boards;

  globalState.GLOBAL_BUS.subscribe((data) => {
    if (boards.message && boards.loader) {
      console.log(data);
      if (data.tag === 'processStatus') {
        //
      }
    }
  });
};
