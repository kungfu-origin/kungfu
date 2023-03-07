import { StartOptions } from 'pm2';
import { I18n } from 'vue-i18n';
import {
  GithubOptions,
  S3Options,
  GenericServerOptions,
} from 'builder-util-runtime';
import { KfHookKeeper } from '../hooks';
import { InstrumentTypeEnum, InstrumentTypes } from './enums';

declare global {
  interface Window {
    watcher: Watcher | null;
    kungfu: Kungfu;
    workers: Record<string, WebpackWorker>;
    basketStore: KungfuApi.BasketStore;
    basketInstrumentStore: KungfuApi.BasketInstrumentStore;
    configStore: KungfuApi.ConfigStore;
    riskSettingStore: KungfuApi.RiskSettingStore;
    commissionStore: KungfuApi.CommissionStore;
    fileId: number;
    testCase: Record<string, any>;
    pm2: any;
  }

  namespace NodeJS {
    interface ProcessEnv {
      LANG_ENV: 'zh-CN' | 'en-US' | 'zh-HK' | undefined;
      APP_TYPE: 'cli' | 'renderer' | 'main' | 'daemon';
      UI_EXT_TYPE: 'component' | 'script';
      APP_ID: string;
      EXTENSION_DIRS: string;
      KFC_DIR: string;
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
      RELOAD_AFTER_CRASHED: 'true' | 'false' | undefined; // 需要作为pm2 env参数传递，为了统一识别，用string
      BY_PASS_RESTORE: boolean;
      BY_PASS_ACCOUNTING: boolean;
      BY_PASS_TRADINGDATA: boolean;
      REFRESH_LEDGER_BEFORE_SYNC: boolean;
      MILLISECONDS_SLEEP_AFTER_STEP: number;
      PM2_DEBUG: boolean
    }

    interface Process {
      resourcesPath: string;
    }
  }
}

declare module 'tail' {
  export class Tail {
    constructor(
      filePath: string,
      options: {
        follow?: boolean;
        fromBeginning?: boolean;
        nLines?: number;
        useWatchFile?: boolean;
      },
    );
    watch(): void;
    unwatch(): void;
    on(type: 'line', callback: (data: string) => void);
    on(type: 'error', callback: (err: Error) => void);
  }
}

declare module globalThis {
  const __publicResources: string;
  const __kfResourcesPath: string;
  const pm2: any;
  const HookKeeper: KfHookKeeper;
  const i18n: I18n;
}

export interface Pm2StartOptions extends StartOptions {
  name: string;
  autorestart?: boolean;
}

export interface T0T1Config {
  T0: {
    instrumentTypes: InstrumentTypes[];
    exchangeIds: string[];
  };
}

export type AllPublishOptions =
  | GithubOptions
  | S3Options
  | GenericServerOptions;

type Writeable<T> = { -readonly [P in keyof T]: T[P] };

export interface RootConfigJSON {
  version?: string;
  kungfuCraft?: {
    appTitle?: string;
    productName?: string;
    autoUpdate?: {
      publish?: Writeable<AllPublishOptions>;
      update?: Writeable<AllPublishOptions>;
    };
  };
  boardFilter?: Record<string, boolean>;
  appConfig?: {
    showHelp?: boolean;

    T0T1?: T0T1Config;

    makeOrder?: {
      priceTypeFilter?: Record<string, boolean>;
      offsetFilter?: Record<string, boolean>;
      ableHedgeFlag?: boolean;
    };
  };
}
