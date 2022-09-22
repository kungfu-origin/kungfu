import { selectTargetKfConfig } from '../assets/methods/utils';
import monitor from '../components/monitor';
import tradingDataMonitor from '../components/tradingDataMonitor';
import { triggerStartStep } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';

// it is so important, because inquirer event will conflict with blessed
process.stdin.removeAllListeners('data');

export const monitPrompt = async (list: boolean) => {
  triggerStartStep();

  if (list) {
    const kfConfig = await selectTargetKfConfig(true);

    if (!kfConfig) {
      throw new Error('target is illegal kfLocation');
    }

    return tradingDataMonitor(kfConfig);
  }
  return monitor();
};
