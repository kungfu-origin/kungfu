declare const __resources: string;
declare const __python_version: string;
declare const __git_commit_version: string;
declare const __build_timestamp: number;
declare var global: typeof globalThis;

declare module NodeJS {
    interface Global {
        __resources: string;
        __kfResourcesPath: string;
    }
}

declare module '@kungfu-trader/kungfu-core' {
    export function kungfu(): {
        longfist: any;
        ConfigStore: any;
        CommissionStore: any;
        History: any;
        watcher: any;
        shutdown: () => void;
    };
}
