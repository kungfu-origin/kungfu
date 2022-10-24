import { Subject } from 'rxjs';
import { StoreDefinition } from 'pinia';
import { Locale } from 'ant-design-vue/es/locale-provider';

declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $antLocalesMap: Record<string, Locale>;
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
