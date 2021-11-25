declare const __resources: string;
declare const __python_version: string;
declare const __git_commit_version: string;
declare const __build_timestamp: number;

declare module NodeJS {
    interface Global {
        __resources: string;
    }
}
