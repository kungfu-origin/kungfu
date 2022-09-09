import { PreStartProcessHooks } from './preStartProcessHook';
import resolveStartOptionsHook from './resolveStartProcessOptionsHook';
import { DealTradingDataHooks } from './dealTradingDataHook';
import resolveExtConfigHook from './resolveExtConfigHook';
import { PrefixHooks } from './prefixHooks';
export interface KfHooks {
  prestart: PreStartProcessHooks;
  resolveStartOptions: typeof resolveStartOptionsHook;
  dealTradingData: DealTradingDataHooks;
  resolveExtConfig: typeof resolveExtConfigHook;
  prefix: PrefixHooks;
}

export class KfHookKeeper {
  hooks: KfHooks;

  constructor() {
    this.hooks = {
      prestart: new PreStartProcessHooks(),
      dealTradingData: new DealTradingDataHooks(),
      resolveStartOptions: resolveStartOptionsHook,
      resolveExtConfig: resolveExtConfigHook,
      prefix: new PrefixHooks(),
    };
  }

  getHooks(): KfHooks {
    return this.hooks;
  }
}

globalThis.HookKeeper = globalThis.HookKeeper || new KfHookKeeper();
