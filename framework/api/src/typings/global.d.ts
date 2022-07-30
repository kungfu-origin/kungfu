import { StartOptions } from 'pm2';
import { KfHookKeeper } from '../hooks';

declare global {
  interface Window {
    watcher: Watcher | null;
    kungfu: Kungfu;
    workers: Record<string, WebpackWorker>;
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
      // DAEMON_ID: string;
      // RENDERER_ID: string;
      EXTENSION_DIRS: string;
      KFC_DIR: string;
      ELECTRON_RUN_AS_NODE: boolean;
      ELECTRON_ENABLE_STACK_DUMPING: boolean;
      RELOAD_AFTER_CRASHED: 'true' | 'false' | undefined; // 需要作为pm2 env参数传递，为了统一识别，用string
      BY_PASS_RESTORE: boolean;
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
}

export interface Pm2StartOptions extends StartOptions {
  name: string;
  autorestart?: boolean;
}
