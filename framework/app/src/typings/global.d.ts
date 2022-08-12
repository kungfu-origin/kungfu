import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $globalBus: Subject<KfBusEvent>;
    $tradingDataSubject: Subject<Watcher>;
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
