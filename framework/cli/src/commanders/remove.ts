import { deleteAllByKfLocation } from '@kungfu-trader/kungfu-js-api/actions';
import { selectTargetKfLocation } from '../assets/methods/utils';

export const removeMdTdStrategy = async () => {
  const kfLocation = await selectTargetKfLocation();
  return deleteAllByKfLocation(kfLocation);
};
