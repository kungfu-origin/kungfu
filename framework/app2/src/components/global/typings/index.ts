export interface BoardInfo {
    children?: number[];
    direction: 'v' | 'h';
    contents?: string[];
    current?: string;
    width?: number;
    height?: number;
}

export interface BoardsMap {
    [prop: number]: BoardInfo;
}
