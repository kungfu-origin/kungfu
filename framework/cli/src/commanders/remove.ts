import { deleteAllByKfLocation } from '@kungfu-trader/kungfu-js-api/actions';
import { selectTargetKfConfig } from '../assets/methods/utils';

export const removeMdTdStrategy = async () => {
  const kfConfig = await selectTargetKfConfig();

  if (!kfConfig) {
    throw new Error('target is illegal kfLocation');
  }

  return deleteAllByKfLocation(kfConfig);
};
