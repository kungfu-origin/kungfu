import { defineStore } from 'pinia';
import { toRaw } from 'vue';
import {
  KfLayoutDirection,
  KfLayoutTargetDirectionClassName,
} from '@root/src/typings/enums';
import {
  getIdByKfLocation,
  getKfExtensionConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  getAllKfConfigOriginData,
  getSubscribedInstruments,
  getTdGroups,
} from '@kungfu-trader/kungfu-js-api/actions';
import {
  Pm2ProcessStatusDetailData,
  Pm2ProcessStatusData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  BrokerStateStatusTypes,
  KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

interface GlobalState {
  boardsMap: KfLayout.BoardsMap;
  dragedContentData: KfLayout.ContentData | null;
  isBoardDragging: boolean;
  extConfigs: KungfuApi.KfExtConfigs;
  tdList: KungfuApi.KfConfig[];
  tdGroupList: KungfuApi.KfExtraLocation[];
  mdList: KungfuApi.KfConfig[];
  strategyList: KungfuApi.KfConfig[];

  processStatusData: Pm2ProcessStatusData;
  processStatusWithDetail: Pm2ProcessStatusDetailData;

  appStates: Record<string, BrokerStateStatusTypes>;
  assets: Record<string, KungfuApi.Asset>;
  instruments: KungfuApi.InstrumentResolved[];
  subscribedInstruments: KungfuApi.InstrumentResolved[];

  currentGlobalKfLocation:
    | KungfuApi.KfLocation
    | KungfuApi.KfConfig
    | KungfuApi.KfExtraLocation
    | null;
}

export const useGlobalStore = defineStore('global', {
  state: (): GlobalState => {
    return {
      boardsMap: {},
      dragedContentData: null,
      isBoardDragging: false,
      extConfigs: toRaw<KungfuApi.KfExtConfigs>({}),

      tdList: [],
      tdGroupList: [],
      mdList: [],
      strategyList: [],

      processStatusData: {},
      processStatusWithDetail: {},

      appStates: {},
      assets: {},
      instruments: [],
      subscribedInstruments: [],

      currentGlobalKfLocation: null,
    };
  },

  actions: {
    setTdGroups() {
      return getTdGroups().then((tdGroups) => {
        this.tdGroupList = tdGroups;
      });
    },

    setSubscribedInstruments() {
      getSubscribedInstruments().then((instruments) => {
        this.subscribedInstruments = toRaw(instruments);
      });
    },

    setInstruments(instruments: KungfuApi.InstrumentResolved[]) {
      this.instruments = toRaw(instruments);
    },

    setCurrentGlobalKfLocation(
      kfLocation:
        | KungfuApi.KfLocation
        | KungfuApi.KfConfig
        | KungfuApi.KfExtraLocation
        | null,
    ) {
      this.currentGlobalKfLocation = kfLocation;
    },

    setAppStates(appStates: Record<string, BrokerStateStatusTypes>) {
      this.appStates = appStates;
    },

    setAssets(assets: Record<string, KungfuApi.Asset>) {
      this.assets = assets;
    },

    setProcessStatus(processStatus: Pm2ProcessStatusData) {
      this.processStatusData = toRaw(processStatus);
    },

    setProcessStatusWithDetail(
      processStatusWithDetail: Pm2ProcessStatusDetailData,
    ) {
      this.processStatusWithDetail = toRaw(processStatusWithDetail);
    },

    setKfConfigList() {
      return getAllKfConfigOriginData().then((res) => {
        const { md, td, strategy } = res;
        this.mdList = md;
        this.tdList = td;
        this.strategyList = strategy;

        if (this.currentGlobalKfLocation === null) {
          if (td.length) {
            this.setCurrentGlobalKfLocation(td[0]);
            return;
          } else if (strategy.length) {
            this.setCurrentGlobalKfLocation(strategy[0]);
          }
        } else if (!this.checkCurrentGlobalKfLocationExisted()) {
          if (this.currentGlobalKfLocation?.category === 'td') {
            if (this.tdList.length) {
              this.setCurrentGlobalKfLocation(td[0]);
              return;
            }
          } else if (this.currentGlobalKfLocation?.category === 'strategy') {
            if (this.strategyList.length) {
              this.setCurrentGlobalKfLocation(strategy[0]);
              return;
            }
          }
        }

        this.setCurrentGlobalKfLocation(null);
      });
    },

    checkCurrentGlobalKfLocationExisted() {
      if (this.currentGlobalKfLocation === null) {
        return false;
      }

      const categoryToKfConfigsMap: Record<
        KfCategoryTypes,
        KungfuApi.KfConfig[]
      > = {
        td: this.tdList,
        md: this.mdList,
        strategy: this.strategyList,
        system: [],
      };

      const targetKfConfigs: KungfuApi.KfConfig[] =
        categoryToKfConfigsMap[
          this.currentGlobalKfLocation.category as KfCategoryTypes
        ];
      if (!targetKfConfigs || !targetKfConfigs.length) {
        return false;
      }

      const afterFilter: KungfuApi.KfConfig[] = targetKfConfigs.filter(
        (item) => {
          if (
            this.currentGlobalKfLocation &&
            getIdByKfLocation(item) ===
              getIdByKfLocation(this.currentGlobalKfLocation)
          ) {
            return true;
          }

          return false;
        },
      );

      return afterFilter.length > 0;
    },

    setKfExtConfigs() {
      return getKfExtensionConfig().then(
        (kfExtConfigs: KungfuApi.KfExtConfigs) => {
          this.extConfigs = toRaw(kfExtConfigs);
        },
      );
    },

    markIsBoardDragging(status: boolean) {
      this.isBoardDragging = status;
    },

    initBoardsMap(boardsMap: KfLayout.BoardsMap) {
      this.boardsMap = boardsMap;
    },

    setBoardsMapAttrById(
      id: number,
      attrKey: keyof KfLayout.BoardInfo,
      value: KfLayout.BoardInfo[keyof KfLayout.BoardInfo],
    ) {
      (<typeof value>this.boardsMap[id][attrKey]) = value;
    },

    removeBoardByContentId(targetBoardId: number, targetContentId: string) {
      const targetBoard: KfLayout.BoardInfo = this.boardsMap[targetBoardId];
      const contents = targetBoard?.contents;
      const targetIndex = contents?.indexOf(targetContentId);

      if (targetIndex === undefined) return;
      if (targetIndex === -1) return;

      const removedItem: KfLayout.ContentId =
        (contents?.splice(targetIndex, 1) || [])[0] || '';

      if (removedItem === targetBoard.current && contents?.length) {
        targetBoard.current = (targetBoard.contents || [])[0];
      }

      if (!contents?.length && targetBoard.paId != -1) {
        this.removeBoardByBoardId_(targetBoardId);
      }
    },

    removeBoardByBoardId_(targetBoardId: number) {
      const targetBoard = this.boardsMap[targetBoardId];
      if (targetBoard && targetBoard.paId !== -1) {
        const paId = targetBoard.paId;
        const paBoard = this.boardsMap[paId];
        const children = paBoard?.children;
        const childIndex = paBoard.children?.indexOf(targetBoardId);

        if (childIndex === undefined) return;
        if (childIndex === -1) return;

        children?.splice(childIndex, 1);

        if (!children?.length) {
          this.removeBoardByBoardId_(paId);
        } else {
          children.forEach((childId: KfLayout.BoardId) => {
            this.boardsMap[childId].width = 0;
            this.boardsMap[childId].height = 0;
          });
        }

        delete this.boardsMap[targetBoardId];
      }
      return;
    },

    setDragedContentData(
      boardId: KfLayout.BoardId,
      contentId: KfLayout.ContentId,
    ) {
      if (boardId === -1 && !contentId) {
        this.dragedContentData = null;
      } else {
        this.dragedContentData = {
          contentId,
          boardId,
        };
      }
    },

    afterDragMoveBoard(
      dragedContentData: KfLayout.ContentData | null,
      destBoardId: KfLayout.BoardId,
      directionClassName: KfLayoutTargetDirectionClassName,
    ) {
      const { boardId, contentId } = dragedContentData || {};
      const destBoard = this.boardsMap[destBoardId];

      if (!contentId || boardId === undefined) return;

      //to self
      if (
        boardId === destBoardId &&
        destBoard.contents &&
        destBoard.contents.length === 1
      ) {
        return;
      }

      this.removeBoardByContentId(boardId, contentId);

      if (directionClassName === KfLayoutTargetDirectionClassName.center) {
        destBoard.contents?.push(contentId);
        destBoard.current = contentId;
      } else if (directionClassName != KfLayoutTargetDirectionClassName.unset) {
        this.dragMakeNewBoard_(contentId, destBoardId, directionClassName);
      }
    },

    dragMakeNewBoard_(
      contentId: KfLayout.ContentId,
      destBoardId: number,
      directionClassName: KfLayoutTargetDirectionClassName,
    ) {
      const destBoard = this.boardsMap[destBoardId];
      const destPaId: number = destBoard.paId;
      const destDirection: KfLayoutDirection = destBoard.direction;
      const newBoardId: KfLayout.BoardId = this.buildNewBoardId_();

      const newBoardDirection: KfLayoutDirection =
        directionClassName === KfLayoutTargetDirectionClassName.top ||
        directionClassName === KfLayoutTargetDirectionClassName.bottom
          ? KfLayoutDirection.h
          : directionClassName === KfLayoutTargetDirectionClassName.left ||
            directionClassName === KfLayoutTargetDirectionClassName.right
          ? KfLayoutDirection.v
          : KfLayoutDirection.unset;
      const newBoardInfo: KfLayout.BoardInfo = {
        paId: destPaId,
        direction: newBoardDirection,
        contents: [contentId],
        current: contentId,
      };

      if (destDirection === newBoardDirection) {
        const siblings = this.boardsMap[destPaId].children;
        const destIndex = siblings?.indexOf(destBoardId);
        if (destIndex === -1 || destIndex === undefined) {
          throw new Error("Insert dest board is not in pa board's childen");
        }

        if (
          directionClassName === KfLayoutTargetDirectionClassName.top ||
          directionClassName === KfLayoutTargetDirectionClassName.left
        ) {
          siblings?.splice(destIndex, 0, newBoardId);
        } else {
          siblings?.splice(destIndex + 1, 0, newBoardId);
        }
      } else {
        newBoardInfo.paId = destBoardId;
        const destBoardCopy: KfLayout.BoardInfo = {
          ...toRaw(destBoard),
          direction: newBoardDirection,
          paId: destBoardId,
          width: undefined,
          height: undefined,
        };

        const newDestBoardId = newBoardId + 1;
        if (
          directionClassName === KfLayoutTargetDirectionClassName.top ||
          directionClassName === KfLayoutTargetDirectionClassName.left
        ) {
          destBoard.children = [newBoardId, newDestBoardId];
        } else {
          destBoard.children = [newDestBoardId, newBoardId];
        }
        delete destBoard.contents;
        delete destBoard.current;

        this.boardsMap[newDestBoardId] = destBoardCopy;
      }

      destBoard.width && delete destBoard.width;
      destBoard.height && delete destBoard.height;

      this.boardsMap[newBoardId] = newBoardInfo;
    },

    buildNewBoardId_(): KfLayout.BoardId {
      const boardIds = Object.keys(this.boardsMap)
        .map((key: string) => +key)
        .sort((key1: number, key2: number) => key2 - key1);
      return boardIds[0] + 1;
    },
  },
});
