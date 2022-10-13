import { PreStartProcessHooks } from './preStartProcessHook';
import resolveStartOptionsHook from './resolveStartProcessOptionsHook';
import { DealTradingDataHooks } from './dealTradingDataHook';
import resolveExtConfigHook from './resolveExtConfigHook';
import { PrefixHooks } from './prefixHooks';
import { DealBoardsMapHook } from './dealBoardsMapHook';
export interface KfHooks {
  prestart: PreStartProcessHooks;
  resolveStartOptions: typeof resolveStartOptionsHook;
  dealTradingData: DealTradingDataHooks;
  resolveExtConfig: typeof resolveExtConfigHook;
  prefix: PrefixHooks;
  dealBoardsMap: DealBoardsMapHook;
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
      dealBoardsMap: new DealBoardsMapHook(),
    };
  }

  getHooks(): KfHooks {
    return this.hooks;
  }
}

globalThis.HookKeeper = globalThis.HookKeeper || new KfHookKeeper();
