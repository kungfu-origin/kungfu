import { PreStartProcessHooks } from './preStartProcessHook';
import { StartProcessHooks } from './startProcessHook';

export interface KfHooks {
  prestart: PreStartProcessHooks;
}

export class KfHookKeeper {
  hooks: KfHooks;

  constructor() {
    this.hooks = {
      prestart: new PreStartProcessHooks(),
      start: new StartProcessHooks(),
    };
  }
}

globalThis.hookKeeper = new KfHookKeeper();
