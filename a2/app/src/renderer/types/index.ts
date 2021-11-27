export type ContentId = string;
export type BoardId = number;
export enum Direction {
    v = 'v',
    h = 'h',
    unset = '',
}

export interface BoardInfo {
    paId: number;
    children?: number[];
    direction: Direction;
    contents?: ContentId[];
    current?: ContentId;
    width?: number | string;
    height?: number | string;
}

export interface BoardsMap {
    [prop: BoardId]: BoardInfo;
}

export interface ContentData {
    contentId: ContentId;
    boardId: BoardId;
}

export enum TargetDirectionClassName {
    unset = '',
    center = 'drag-over',
    top = 'drag-over-top',
    bottom = 'drag-over-bottom',
    left = 'drag-over-left',
    right = 'drag-over-right',
}
