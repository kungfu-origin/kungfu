import { PreStartProcessHooks } from './preStartProcessHook';
import { ResolveStartOptionsHooks } from './resolveStartProcessOptionsHook';
import { DealTradingDataHooks } from './dealTradingDataHook';
export interface KfHooks {
  prestart: PreStartProcessHooks;
  resolveStartOptions: ResolveStartOptionsHooks;
  dealTradingData: DealTradingDataHooks;
}

export class KfHookKeeper {
  hooks: KfHooks;

  constructor() {
    this.hooks = {
      prestart: new PreStartProcessHooks(),
      resolveStartOptions: new ResolveStartOptionsHooks(),
      dealTradingData: new DealTradingDataHooks(),
    };
  }

  getHooks(): KfHooks {
    return this.hooks;
  }
}

globalThis.HookKeeper = new KfHookKeeper();
