import { App, Component, computed } from 'vue';

export const basketTradeBoardsMapStorageKey = 'basketTradeBoardsMap';

export const getBasketTradeBoardsMap = (): KfLayout.BoardsMap | null => {
  const data = localStorage.getItem(basketTradeBoardsMapStorageKey);
  if (!data) {
    return null;
  }

  const storedBoardsMap = JSON.parse(data) as KfLayout.BoardsMap;
  if (!Object.keys(storedBoardsMap).length) {
    return null;
  }

  return storedBoardsMap;
};

export const setBasketTradeBoardsMap = (boardsMap: KfLayout.BoardsMap) => {
  return localStorage.setItem(
    basketTradeBoardsMapStorageKey,
    JSON.stringify(boardsMap),
  );
};

export const registerComponents = (
  app: App,
  componentsMap: { [componentsName: string]: Component },
) => {
  Object.keys(componentsMap).forEach((componentsName) => {
    app.component(componentsName, componentsMap[componentsName]);
  });
};

export const buildTableColumnSorter = <T>(
  type: 'num' | 'str',
  dataIndex: keyof T,
) => {
  if (type === 'num') {
    return (a: T, b: T) =>
      Number(a[dataIndex] ?? 0) - Number(b[dataIndex] ?? 0);
  } else {
    return (a: T, b: T) =>
      `${a[dataIndex] ?? ''}`.localeCompare(`${b[dataIndex] ?? ''}`);
  }
};

export const buildConfigGetterWrapByComputed = <
  T extends (
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    ...args: any[]
  ) => Array<
    KfTradingDataTableHeaderConfig | AntTableColumn | KungfuApi.KfConfigItem
  >,
>(
  defaultColumnsGetter: T,
) => {
  return (...args: Parameters<T>) =>
    computed(() => {
      return defaultColumnsGetter(...args) as ReturnType<T>;
    });
};
