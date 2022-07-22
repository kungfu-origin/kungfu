import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { PrefixRegister } from '@kungfu-trader/kungfu-js-api/utils/prefixUtils';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $globalBus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
    $prefixRegister: PrefixRegister;
    $availKfBoards: string[];
    _: any; //vue3 shit
  }
}

declare module 'worker-loader!*' {
  class WebpackWorker extends Worker {
    constructor();
  }

  export default WebpackWorker;
}

export {};
