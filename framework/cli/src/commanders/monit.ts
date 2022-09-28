import { selectTargetKfConfig } from '../assets/methods/utils';
import monitor from '../components/monitor';
import tradingDataMonitor from '../components/tradingDataMonitor';
import { globalState } from '../assets/actions/globalState';
import {
  dealAppStates,
  setTimerPromiseTask,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

// it is so important, because inquirer event will conflict with blessed
process.stdin.removeAllListeners('data');

export const monitPrompt = async (list: boolean) => {
  const { watcher } = await import(
    '@kungfu-trader/kungfu-js-api/kungfu/watcher'
  );
  const { triggerStartStep } = await import(
    '@kungfu-trader/kungfu-js-api/kungfu/tradingData'
  );

  triggerStartStep();

  setTimerPromiseTask((): Promise<void> => {
    return new Promise((resolve) => {
      globalState.APP_STATES_SUBJECT.next(
        dealAppStates(watcher, watcher?.appStates || {}),
      );
      resolve();
    });
  }, 3000);

  const timer = setTimeout(() => {
    globalState.APP_STATES_SUBJECT.next({});
    clearTimeout(timer);
  }, 1000);

  if (list) {
    const kfConfig = await selectTargetKfConfig(true);

    if (!kfConfig) {
      throw new Error('target is illegal kfLocation');
    }

    return tradingDataMonitor(kfConfig);
  }
  return monitor();
};
