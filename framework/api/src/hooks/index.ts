import { PreStartProcessHooks } from './preStartProcessHook';
import resolveStartOptionsHook from './resolveStartProcessOptionsHook';
import { DealTradingDataHooks } from './dealTradingDataHook';
import resolveExtConfigHook from './resolveExtConfigHook';
export interface KfHooks {
  prestart: PreStartProcessHooks;
  resolveStartOptions: typeof resolveStartOptionsHook;
  dealTradingData: DealTradingDataHooks;
  resolveExtConfig: typeof resolveExtConfigHook;
}

export class KfHookKeeper {
  hooks: KfHooks;

  constructor() {
    this.hooks = {
      prestart: new PreStartProcessHooks(),
      dealTradingData: new DealTradingDataHooks(),
      resolveStartOptions: resolveStartOptionsHook,
      resolveExtConfig: resolveExtConfigHook,
    };
  }

  getHooks(): KfHooks {
    return this.hooks;
  }
}

globalThis.HookKeeper = new KfHookKeeper();
