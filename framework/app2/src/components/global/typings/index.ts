export type ContentId = string;
export type BoardId = number;

export interface BoardInfo {
    paId: number;
    children?: number[];
    direction: 'v' | 'h';
    contents?: ContentId[];
    current?: ContentId;
    width?: number;
    height?: number;
}

export interface BoardsMap {
    [prop: BoardId]: BoardInfo;
}
