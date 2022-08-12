import { Pm2StartOptions } from '../typings/global';
import { ResetOptionHook } from './baseHook';

export type ResolveStartOptionsHook = (
  kfLocation: KungfuApi.DerivedKfLocation,
  options: Pm2StartOptions,
) => Promise<Pm2StartOptions>;

export default new ResetOptionHook<ResolveStartOptionsHook, Pm2StartOptions>(
  'ResolveStartOptionsHook',
);
