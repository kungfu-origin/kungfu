export interface BoardInfo {
    children?: number[];
    direction: string;
    contents?: string[];
    current?: string;
}

export interface BoradsMap {
    [prop: number]: BoardInfo;
}
