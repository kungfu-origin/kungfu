import { ResetOptionHook } from './baseHook';

export type ResolveExtConfigHook = (
  kfLocation: KungfuApi.DerivedKfLocation,
  extConfig: KungfuApi.KfExtConfig,
) => KungfuApi.KfExtConfig;

export default new ResetOptionHook<ResolveExtConfigHook, KungfuApi.KfExtConfig>(
  'ResolveExtConfigHook',
);
