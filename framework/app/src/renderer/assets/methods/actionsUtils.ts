import os from 'os';
import { dialog, shell } from '@electron/remote';
import { ensureRemoveLocation } from '@kungfu-trader/kungfu-js-api/actions';
import {
  dealPosition,
  dealTradingDataItem,
  getKungfuHistoryData,
  hashInstrumentUKey,
  kfRequestMarketData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
  BrokerStateStatusTypes,
  DirectionEnum,
  HistoryDateEnum,
  InstrumentTypeEnum,
  InstrumentTypes,
  KfCategoryTypes,
  OffsetEnum,
  PriceTypeEnum,
  ProcessStatusTypes,
  SideEnum,
  StrategyExtTypes,
  OrderInputKeyEnum,
  LedgerCategoryEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  getKfCategoryData,
  getIdByKfLocation,
  getMdTdKfLocationByProcessId,
  getProcessIdByKfLocation,
  switchKfLocation,
  findTargetFromArray,
  getAppStateStatusName,
  buildExtTypeMap,
  dealCategory,
  dealAssetsByHolderUID,
  getAvailDaemonList,
  getStrategyStateStatusName,
  isBrokerStateReady,
  dealKfNumber,
  dealKfPrice,
  transformSearchInstrumentResultToInstrument,
  booleanProcessEnv,
  isShotable,
  isT0,
  getTradingDataSortKey,
  isUpdateVersionLogicEnable,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { BasketVolumeType } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { writeCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import {
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { message, Modal } from 'ant-design-vue';
import path from 'path';
import { Proc } from 'pm2';
import {
  computed,
  ComputedRef,
  getCurrentInstance,
  h,
  nextTick,
  onBeforeUnmount,
  onMounted,
  reactive,
  ref,
  Ref,
  toRaw,
  toRefs,
  watch,
} from 'vue';
import dayjs from 'dayjs';
import { Row } from '@fast-csv/format';
import {
  AbleSubscribeInstrumentTypesBySourceType,
  OrderInputKeySetting,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  buildInstrumentSelectOptionLabel,
  buildInstrumentSelectOptionValue,
  confirmModal,
  makeSearchOptionFormInstruments,
} from './uiUtils';
import { storeToRefs } from 'pinia';
import { ipcRenderer } from 'electron';
import { throttleTime } from 'rxjs';
import { useGlobalStore } from '../../pages/index/store/global';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import sound from 'sound-play';
import { KUNGFU_RESOURCES_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { RuleObject } from 'ant-design-vue/lib/form';
import { TradeAccountingUsageMap } from '@kungfu-trader/kungfu-js-api/utils/accounting';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();

export const useUpdateVersion = () => {
  const vueInstance = getCurrentInstance();
  const packageJson = readRootPackageJsonSync();
  const currentVersion = ref(packageJson?.version);
  const newVersion = ref('');
  const popoverVisible = ref(false);
  const hasNewVersion = ref(false);
  const downloadStarted = ref<boolean>(false);
  const progressStatus = ref<'success' | 'active' | 'exception' | 'normal'>(
    'normal',
  );
  const errorMessage = ref('');
  const process = ref<number>();

  const handleToConfirmStartUpdate = (newVersion: string) => {
    confirmModal(
      t('globalSettingConfig.update'),
      t('globalSettingConfig.find_new_version', {
        version: newVersion,
      }),
    ).then((flag) => {
      ipcRenderer.send('auto-update-confirm-result', flag);
    });
  };

  const handleToStartDownload = () => {
    ipcRenderer.send('auto-update-to-start-download');
  };

  const handleQuitAndInstall = () => {
    confirmModal(
      t('globalSettingConfig.update'),
      t('globalSettingConfig.warning_before_install'),
    ).then((flag) => {
      if (flag) {
        ipcRenderer.send('auto-update-quit-and-install');
      }
    });
  };

  onMounted(() => {
    if (!isUpdateVersionLogicEnable()) return;

    vueInstance?.proxy?.$globalBus.subscribe((data) => {
      if (data.tag === 'main') {
        if (data.name === 'auto-update-find-new-version') {
          hasNewVersion.value = true;
          newVersion.value = data.payload.newVersion;
          handleToConfirmStartUpdate(data.payload.newVersion);
        }

        if (data.tag === 'auto-update-up-to-date') {
          hasNewVersion.value = false;
        }

        if (data.name === 'auto-update-start-download') {
          downloadStarted.value = true;
          progressStatus.value = 'active';
          popoverVisible.value = true;
        }

        if (data.name === 'auto-update-download-process') {
          process.value = +data.payload.process;
          if (process.value === 100) {
            progressStatus.value = 'success';
          } else {
            progressStatus.value = 'active';
          }
        }

        if (data.name === 'auto-update-error') {
          errorMessage.value = (data.payload.error as Error).message;
          progressStatus.value = 'exception';
        }
      }
    });
  });

  return {
    popoverVisible,
    newVersion,
    currentVersion,
    hasNewVersion,
    downloadStarted,
    process,
    progressStatus,
    errorMessage,
    handleToStartDownload,
    handleQuitAndInstall,
  };
};

export const handleSwitchProcessStatusGenerator = (): ((
  checked: boolean,
  mouseEvent: MouseEvent,
  kfLocation: KungfuApi.KfLocation,
) => Promise<void | Proc>) => {
  const switchController = {};
  return (
    checked: boolean,
    mouseEvent: MouseEvent,
    kfLocation: KungfuApi.KfLocation,
  ) =>
    handleSwitchProcessStatus(
      checked,
      mouseEvent,
      kfLocation,
      switchController,
    );
};

export const handleSwitchProcessStatus = (
  checked: boolean,
  mouseEvent: MouseEvent,
  kfLocation: KungfuApi.KfLocation,
  switchController: Record<string, boolean>,
): Promise<void | Proc> => {
  mouseEvent.stopPropagation();
  const processId = getProcessIdByKfLocation(kfLocation);
  if (switchController[processId]) {
    message.warn(t('please_wait'));
    return Promise.resolve();
  }

  switchController[processId] = true;
  return switchKfLocation(window.watcher, kfLocation, checked)
    .then(() => {
      success();
    })
    .catch((err: Error) => {
      error(err.message || t('operation_failed'));
    })
    .finally(() => {
      switchController[processId] = false;
    });
};

export const preQuitTasks = (tasks: Promise<void>[]): Promise<[]> => {
  return Promise.all(tasks).then(() => {
    return [];
  });
};

export const useSwitchAllConfig = (
  kfConfigs: Ref<KungfuApi.KfConfig[]> | Ref<KungfuApi.KfLocation[]>,
  processStatusData: Ref<Pm2ProcessStatusData>,
): {
  allProcessOnline: ComputedRef<boolean>;
  handleSwitchAllProcessStatus(checked: boolean): Promise<void>;
} => {
  const allProcessOnline = computed(() => {
    const onlineItemsCount: number = kfConfigs.value.filter(
      (item: KungfuApi.KfLocation | KungfuApi.KfConfig): boolean => {
        const processId = getProcessIdByKfLocation(item);
        return processStatusData.value[processId] === 'online';
      },
    ).length;
    if (
      onlineItemsCount === kfConfigs.value.length &&
      kfConfigs.value.length !== 0
    ) {
      return true;
    } else {
      return false;
    }
  });

  const handleSwitchAllProcessStatus = (checked: boolean): Promise<void> => {
    return Promise.all(
      kfConfigs.value.map(
        (item: KungfuApi.KfLocation): Promise<void | Proc> => {
          return switchKfLocation(window.watcher, item, checked);
        },
      ),
    )
      .then(() => {
        success();
      })
      .catch((err: Error) => {
        error(err.message || t('operation_failed'));
      });
  };

  return {
    allProcessOnline,
    handleSwitchAllProcessStatus,
  };
};

export const useAddUpdateRemoveKfConfig = (): {
  handleRemoveKfConfig: (
    watcher: KungfuApi.Watcher,
    kfConfig: KungfuApi.KfConfig | KungfuApi.KfLocation,
    processStatusData: Pm2ProcessStatusData,
  ) => Promise<void>;
  handleConfirmAddUpdateKfConfig: (
    data: {
      formState: Record<string, KungfuApi.KfConfigValue>;
      configSettings: KungfuApi.KfConfigItem[];
      idByPrimaryKeys: string;
      changeType: KungfuApi.ModalChangeType;
    },
    category: KfCategoryTypes,
    group: string,
  ) => Promise<void>;
} => {
  const handleRemoveKfConfig = (
    watcher: KungfuApi.Watcher,
    kfConfig: KungfuApi.KfConfig | KungfuApi.KfLocation,
    processStatusData: Pm2ProcessStatusData,
  ): Promise<void> => {
    const categoryName = getKfCategoryData(kfConfig.category).name;
    const id = getIdByKfLocation(kfConfig);
    return new Promise((resolve, reject) => {
      Modal.confirm({
        title: `${t('delete')}${categoryName} ${id}`,
        content: t('delete_category', {
          category: `${categoryName} ${id}`,
          categoryName: categoryName,
        }),
        okText: t('confirm'),
        cancelText: t('cancel'),
        onOk() {
          return ensureRemoveLocation(watcher, kfConfig, processStatusData)
            .then(() => {
              return useGlobalStore().setKfConfigList();
            })
            .then(() => {
              resolve();
            })
            .catch((err) => {
              reject(err);
            });
        },
      });
    });
  };

  const handleConfirmAddUpdateKfConfig = (
    data: {
      formState: Record<string, KungfuApi.KfConfigValue>;
      configSettings: KungfuApi.KfConfigItem[];
      idByPrimaryKeys: string;
      changeType: KungfuApi.ModalChangeType;
    },
    category: KfCategoryTypes,
    group: string,
  ): Promise<void> => {
    const { formState, idByPrimaryKeys, changeType } = data;

    const changeTypename = changeType === 'add' ? t('add') : t('set');
    const categoryName = getKfCategoryData(category).name;

    const context =
      changeType === 'add'
        ? t('add_config_modal', {
            category: categoryName,
            changeTypename: changeTypename,
            key: `${changeTypename} ${idByPrimaryKeys}`,
          })
        : t('update_config_modal', {
            key: `${changeTypename} ${idByPrimaryKeys}`,
          });
    return new Promise((resolve) => {
      Modal.confirm({
        title: `${changeTypename}${categoryName} ${idByPrimaryKeys}`,
        content: context,
        okText: t('confirm'),
        cancelText: t('cancel'),
        onOk() {
          const kfLocation: KungfuApi.KfLocation = {
            category: category,
            group: group,
            name: idByPrimaryKeys.toString(),
            mode: 'live',
          };

          return setKfConfig(
            kfLocation,
            JSON.stringify({
              ...formState,
              add_time: +new Date().getTime() * Math.pow(10, 6),
            }),
          )
            .then(() => {
              success();
            })
            .then(() => {
              useGlobalStore().setKfConfigList();
            })
            .catch((err: Error) => {
              error(t('operation_failed') + err.message);
            })
            .finally(() => {
              resolve();
            });
        },
        onCancel() {
          resolve();
        },
      });
    });
  };

  return {
    handleRemoveKfConfig,
    handleConfirmAddUpdateKfConfig,
  };
};

export const useDealExportHistoryTradingData = (): {
  exportDateModalVisible: Ref<boolean>;
  exportDataLoading: Ref<boolean>;
  exportEventData: Ref<KfEvent.ExportTradingDataEvent | undefined>;
  handleConfirmExportDate(formSate: {
    date: string;
    dateType: HistoryDateEnum;
  }): void;
} => {
  const app = getCurrentInstance();
  const exportDateModalVisible = ref<boolean>(false);
  const exportEventData = ref<KfEvent.ExportTradingDataEvent>();
  const exportDataLoading = ref<boolean>(false);

  const dealTradingDataItemResolved = (
    isShowOriginData = false,
  ): ((item: KungfuApi.TradingDataTypes) => Row) => {
    return (item) =>
      dealTradingDataItem(item, window.watcher, isShowOriginData) as Row;
  };

  const handleConfirmExportDate = async (formState: {
    date: string;
    dateType: HistoryDateEnum;
  }): Promise<void> => {
    if (!exportEventData.value) {
      throw new Error('exportEventData is undefined');
    }

    const { currentKfLocation, tradingDataType } =
      exportEventData.value || ({} as KfEvent.ExportTradingDataEvent);
    const { date, dateType } = formState;
    const dateResolved = dayjs(date).format('YYYYMMDD');

    if (tradingDataType === 'all') {
      let historyData: {
        tradingData: KungfuApi.TradingData;
      } | null = null;

      try {
        historyData = await getKungfuHistoryData(
          date,
          dateType,
          tradingDataType,
        );
      } catch (err) {
        if (err instanceof Error) {
          if (err.message === 'database_locked') {
            error(t('database_locked'));
          } else {
            console.error(err);
          }
        } else {
          console.error(err);
        }
      }

      if (!historyData) return;

      const { tradingData } = historyData;
      const orderSortKey = getTradingDataSortKey('Order');
      const orders = tradingData.Order.sort(orderSortKey);
      const tradeSortKey = getTradingDataSortKey('Trade');
      const trades = tradingData.Trade.sort(tradeSortKey);
      const orderStatSortKey = getTradingDataSortKey('OrderStat');
      const orderStat = tradingData.OrderStat.sort(orderStatSortKey);
      const positionSortKey = getTradingDataSortKey('Position');
      const positions = tradingData.Position.sort(positionSortKey);
      const assetSortKey = getTradingDataSortKey('Asset');
      const assets = tradingData.Asset.sort(assetSortKey);

      const { filePaths } = await dialog.showOpenDialog({
        properties: ['openDirectory'],
      });

      if (!filePaths) {
        return;
      }

      const targetFolder = filePaths[0];

      const ordersFilename = path.join(
        targetFolder,
        `orders-${dateResolved}.csv`,
      );
      const tradesFilename = path.join(
        targetFolder,
        `trades-${dateResolved}.csv`,
      );
      const orderStatFilename = path.join(
        targetFolder,
        `orderStats-${dateResolved}.csv`,
      );
      const posFilename = path.join(targetFolder, `pos-${dateResolved}.csv`);
      const assetFilename = path.join(
        targetFolder,
        `assets-${dateResolved}.csv`,
      );

      return Promise.all([
        writeCSV(ordersFilename, orders, dealTradingDataItemResolved()),
        writeCSV(tradesFilename, trades, dealTradingDataItemResolved()),
        writeCSV(
          orderStatFilename,
          orderStat,
          dealTradingDataItemResolved(true),
        ),
        writeCSV(posFilename, positions, dealTradingDataItemResolved()),
        writeCSV(assetFilename, assets, dealTradingDataItemResolved()),
      ])
        .then(() => {
          shell.showItemInFolder(ordersFilename);
          success();
        })
        .catch((err: Error) => {
          error(err.message);
        });
    }

    if (!currentKfLocation) {
      return;
    }

    exportDataLoading.value = true;
    let historyData: {
      tradingData: KungfuApi.TradingData;
    } | null = null;

    try {
      historyData = await getKungfuHistoryData(
        date,
        dateType,
        tradingDataType,
        currentKfLocation,
      );
    } catch (err) {
      if (err instanceof Error) {
        if (err.message === 'database_locked') {
          error(t('database_locked'));
        } else {
          console.error(err);
        }
      } else {
        console.error(err);
      }
    }

    exportDataLoading.value = false;

    if (!historyData) return Promise.resolve();

    const { tradingData } = historyData;

    const processId = getProcessIdByKfLocation(currentKfLocation);
    const filename: string = await dialog
      .showSaveDialog({
        title: t('save_file'),
        defaultPath: path.join(
          os.homedir(),
          `${processId}-${tradingDataType}-${dateResolved}.csv`,
        ),
        filters: [
          {
            name: 'csv',
            extensions: ['csv'],
          },
        ],
      })
      .then(({ filePath }) => {
        return filePath || '';
      });

    if (!filename) {
      return Promise.resolve();
    }

    const exportDatas =
      globalThis.HookKeeper.getHooks().dealTradingData.trigger(
        window.watcher,
        currentKfLocation,
        tradingData[tradingDataType as KungfuApi.TradingDataTypeName] as
          | KungfuApi.DataTable<KungfuApi.Order>
          | KungfuApi.DataTable<KungfuApi.Trade>
          | KungfuApi.DataTable<KungfuApi.Position>,
        tradingDataType.toLowerCase(),
      );

    return writeCSV(filename, exportDatas, dealTradingDataItemResolved())
      .then(() => {
        shell.showItemInFolder(filename);
        success();
      })
      .catch((err: Error) => {
        error(err.message);
      });
  };

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$globalBus.subscribe(
        (data: KfEvent.KfBusEvent) => {
          if (data.tag === 'export') {
            exportEventData.value = data;

            if (!exportEventData.value) return;

            if (exportEventData.value.tradingDataType !== 'all') {
              if (exportEventData.value.tradingDataType !== 'Order') {
                if (exportEventData.value.tradingDataType !== 'Trade') {
                  if (exportEventData.value.tradingDataType !== 'OrderInput') {
                    handleConfirmExportDate({
                      date: dayjs().format(),
                      dateType: HistoryDateEnum.naturalDate,
                    });
                    return;
                  }
                }
              }
            }

            exportDateModalVisible.value = true;
          }
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    exportDateModalVisible,
    exportDataLoading,
    exportEventData,
    handleConfirmExportDate,
  };
};

export const showTradingDataDetail = (
  item: KungfuApi.TradingDataTypes,
  typename: string,
): Promise<boolean> => {
  const dataResolved = dealTradingDataItem(item, window.watcher);
  const vnode = Object.keys(dataResolved || {})
    .filter((key) => {
      if (dataResolved[key].toString() === '[object Object]') {
        return false;
      }
      return dataResolved[key] !== '';
    })
    .map((key) =>
      h('div', { class: 'trading-data-detail-row' }, [
        h('span', { class: 'label' }, `${key}`),
        h('span', { class: 'value' }, `${dataResolved[key]}`),
      ]),
    );

  return confirmModal(
    `${typename} ${t('detail')}`,
    h(
      'div',
      {
        class: 'trading-data-detail__warp',
      },
      vnode,
    ),
    t('confirm'),
    '',
  );
};

export const useInstruments = (): {
  instruments: Ref<KungfuApi.InstrumentResolved[]>;
  subscribedInstrumentsByLocal: Ref<KungfuApi.InstrumentResolved[]>;
  subscribeAllInstrumentByMdProcessId(
    processId: string,
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ): Promise<Array<KungfuApi.InstrumentResolved>>;
  subscribeAllInstrumentByAppStates(
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ): Promise<Array<KungfuApi.InstrumentResolved>>;

  searchInstrumentResult: Ref<string | undefined>;
  searchInstrumnetOptions: Ref<{ value: string; label: string }[]>;
  updateSearchInstrumnetOptions: (
    type: 'instrument' | 'instruments' | 'instrumentsCsv',
    value: string | string[],
  ) => Promise<{ value: string; label: string }[]>;
  handleSearchInstrument: (
    value: string,
  ) => Promise<{ value: string; label: string }[]>;
  handleSearchByCustom: (
    value: string,
    customOptions: {
      customInstruments?: KungfuApi.InstrumentResolved[];
      customFilterCondition?: (
        keywords: string,
        pos: KungfuApi.InstrumentResolved,
      ) => boolean;
    },
  ) => Promise<{ value: string; label: string }[]>;
  handleConfirmSearchInstrumentResult: (
    value: string,
    callback?: (value: string) => void,
  ) => void;
} => {
  const { instruments, subscribedInstrumentsByLocal } = storeToRefs(
    useGlobalStore(),
  );

  const subscribeAllInstrumentByMdProcessId = async (
    processId: string,
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentForSub[],
  ): Promise<Array<KungfuApi.InstrumentForSub>> => {
    if (isBrokerStateReady(appStates[processId])) {
      if (processStatus[processId] === 'online') {
        if (processId.indexOf('md_') === 0) {
          const mdLocation = getMdTdKfLocationByProcessId(processId);
          if (mdLocation && mdLocation.category === 'md') {
            const sourceId = mdLocation.group;
            const sourceType = mdExtTypeMap[sourceId];
            const ableSubscribedInstrumentTypes =
              AbleSubscribeInstrumentTypesBySourceType[sourceType] || [];

            const instrumentsForSubscribeResolved =
              instrumentsForSubscribe.filter((item) =>
                ableSubscribedInstrumentTypes.includes(+item.instrumentType),
              );
            const subscribeResults = await Promise.all(
              instrumentsForSubscribeResolved.map((item) =>
                kfRequestMarketData(
                  window.watcher,
                  item.exchangeId,
                  item.instrumentId,
                  mdLocation,
                ),
              ),
            );
            return instrumentsForSubscribeResolved.filter(
              (_, index) => !!subscribeResults[index],
            );
          }
        }
      }
    }

    return [];
  };

  const subscribeAllInstrumentByAppStates = async (
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentForSub[],
  ): Promise<Array<KungfuApi.InstrumentForSub>> => {
    const subscribedSuccessInstruments = await Promise.all(
      Object.keys(appStates || {}).map((processId) =>
        subscribeAllInstrumentByMdProcessId(
          processId,
          processStatus,
          appStates,
          mdExtTypeMap,
          instrumentsForSubscribe,
        ),
      ),
    );

    return subscribedSuccessInstruments.reduce((pre, instruments) => {
      return pre.concat(instruments);
    }, []);
  };

  const searchInstrumentResult = ref<string | undefined>(undefined);
  const searchInstrumnetOptions = ref<{ value: string; label: string }[]>([]);

  const updateSearchInstrumnetOptions = (
    type: 'instrument' | 'instruments' | 'instrumentsCsv',
    value: string | string[],
  ): Promise<{ value: string; label: string }[]> => {
    searchInstrumnetOptions.value = makeSearchOptionFormInstruments(
      type,
      value,
    );
    return Promise.resolve(searchInstrumnetOptions.value);
  };

  const filterInstrumentsByKeyword = (
    keywords: string,
    curInstruments: KungfuApi.InstrumentResolved[],
    filterCondition: (
      keywords: string,
      pos: KungfuApi.InstrumentResolved,
    ) => boolean,
  ) => {
    return curInstruments
      .filter((item) => filterCondition(keywords, item))
      .slice(0, 20)
      .map((item) => ({
        value: buildInstrumentSelectOptionValue(item),
        label: buildInstrumentSelectOptionLabel(item),
      }));
  };

  const defaultFilterCondition = (
    keywords: string,
    pos: KungfuApi.InstrumentResolved,
  ) => {
    const regx = new RegExp(`${keywords}`, 'ig');
    return !!keywords && regx.test(pos.id);
  };

  const handleSearchByCustom = (
    val: string,
    customOptions: {
      customInstruments?: KungfuApi.InstrumentResolved[];
      customFilterCondition?: (
        keywords: string,
        pos: KungfuApi.InstrumentResolved,
      ) => boolean;
    },
  ) => {
    const { customInstruments, customFilterCondition } = customOptions;
    const customInstrumentOptions = filterInstrumentsByKeyword(
      val,
      customInstruments || instruments.value,
      customFilterCondition || defaultFilterCondition,
    );
    return Promise.resolve(customInstrumentOptions);
  };

  const handleSearchInstrument = async (
    val: string,
  ): Promise<{ value: string; label: string }[]> => {
    searchInstrumnetOptions.value = await handleSearchByCustom(val, {
      customInstruments: instruments.value,
      customFilterCondition: defaultFilterCondition,
    });
    return Promise.resolve(searchInstrumnetOptions.value);
  };

  const handleConfirmSearchInstrumentResult = (
    value: string,
    callback?: (value: string) => void,
  ) => {
    nextTick().then(() => {
      searchInstrumentResult.value = undefined;
    });
    callback && callback(value);
  };

  return {
    instruments,
    subscribedInstrumentsByLocal,
    subscribeAllInstrumentByMdProcessId,
    subscribeAllInstrumentByAppStates,

    searchInstrumentResult,
    searchInstrumnetOptions,
    updateSearchInstrumnetOptions,
    handleSearchByCustom,
    handleSearchInstrument,
    handleConfirmSearchInstrumentResult,
  };
};

export const usePreStartAndQuitApp = (): {
  preStartSystemLoadingData: Record<string, 'loading' | 'done'>;
  preStartSystemLoading: ComputedRef<boolean>;
  preQuitSystemLoadingData: Record<string, 'loading' | 'done' | undefined>;
  preQuitSystemLoading: ComputedRef<boolean>;
  saveBoardsMap: () => Promise<void>;
} => {
  const app = getCurrentInstance();

  const preStartSystemLoadingData = reactive<
    Record<string, 'loading' | 'done'>
  >({
    archive: 'loading',
    watcher: 'loading',
    extraResourcesLoading: 'loading',
  });

  const preQuitSystemLoadingData = reactive<
    Record<string, 'loading' | 'done' | undefined>
  >({
    record: undefined,
    quit: undefined,
  });

  const preStartSystemLoading = computed(() => {
    return (
      Object.values(preStartSystemLoadingData).filter(
        (item: string) => item !== 'done',
      ).length > 0
    );
  });

  const watchStopHandle = watch(
    () => preStartSystemLoading.value,
    (newVal) => {
      if (!newVal) {
        ipcRenderer.send('auto-update-renderer-ready');
        watchStopHandle();
      }
    },
  );

  const preQuitSystemLoading = computed(() => {
    return (
      Object.values(preQuitSystemLoadingData).filter(
        (item: string | undefined) => item !== undefined,
      ).length > 0
    );
  });

  const startGetWatcherStatus = () => {
    const timer = setInterval(() => {
      if (window.watcher?.isLive()) {
        preStartSystemLoadingData.watcher = 'done';
        clearInterval(timer);
      } else {
        preStartSystemLoadingData.watcher = 'loading';
      }
    }, 500);
  };

  startGetWatcherStatus();

  const saveBoardsMap = (): Promise<void> => {
    const { boardsMap } = storeToRefs(useGlobalStore());
    localStorage.setItem(
      'indexBoardsMap',
      JSON.stringify(boardsMap.value || '{}'),
    );
    return Promise.resolve();
  };

  onMounted(() => {
    if (booleanProcessEnv(process.env.RELOAD_AFTER_CRASHED)) {
      preStartSystemLoadingData.archive = 'done';
      preStartSystemLoadingData.extraResourcesLoading = 'done';
    }

    if (app?.proxy) {
      const subscription = app?.proxy.$globalBus.subscribe(
        (data: KfEvent.KfBusEvent) => {
          if (data.tag === 'processStatus') {
            if (data.name && data.name === 'archive') {
              preStartSystemLoadingData.archive =
                data.status === 'online' ? 'loading' : 'done';
              startGetWatcherStatus();
            }

            if (data.name && data.name === 'extraResourcesLoading') {
              preStartSystemLoadingData.extraResourcesLoading =
                data.status === 'online' ? 'done' : 'loading';
            }

            if (data.name === 'system' && data.status === 'waiting restart') {
              preStartSystemLoadingData.archive = 'loading';
              preStartSystemLoadingData.watcher = 'loading';
              preStartSystemLoadingData.extraResourcesLoading = 'loading';
            }
          }

          if (data.tag === 'main') {
            switch (data.name) {
              case 'record-before-quit':
                preQuitSystemLoadingData.record = 'loading';
                preQuitTasks([
                  // removeNoDefaultStrategyFolders(),
                  Promise.resolve(),
                ]).finally(() => {
                  ipcRenderer.send('record-before-quit-done');
                  preQuitSystemLoadingData.record = 'done';
                });
                break;
              case 'clear-process-before-quit-start':
                preQuitSystemLoadingData.quit = 'loading';
                break;
              case 'clear-process-before-quit-end':
                preQuitSystemLoadingData.quit = 'done';
                break;
            }
          }
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    preStartSystemLoadingData,
    preStartSystemLoading,
    preQuitSystemLoadingData,
    preQuitSystemLoading,
    saveBoardsMap,
  };
};

export const useSubscibeInstrumentAtEntry = (
  watcher: KungfuApi.Watcher | null,
  customInstrumentsForSubGetter?: (
    watcher: KungfuApi.Watcher,
  ) => KungfuApi.InstrumentForSub[],
): void => {
  const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(watcher);
  const { appStates, processStatusData } = useProcessStatusDetailData();
  const { mdExtTypeMap } = useExtConfigsRelated();
  const { subscribedInstrumentsByLocal } = useInstruments();
  const { subscribeAllInstrumentByAppStates } = useInstruments();
  const { curSubscribedInstruments, setCurSubscribedInstruments } =
    useGlobalStore();

  const app = getCurrentInstance();
  const SUBSCRIBE_INSTRUMENTS_LIMIT = 50;

  const getCurrentPositionsForSub = (watcher: KungfuApi.Watcher) => {
    if (!currentGlobalKfLocation.value) return [];

    const positions = globalThis.HookKeeper.getHooks().dealTradingData.trigger(
      watcher,
      currentGlobalKfLocation.value,
      watcher.ledger.Position,
      'position',
    ) as KungfuApi.Position[];

    return positions
      .reverse()
      .slice(0, SUBSCRIBE_INSTRUMENTS_LIMIT)
      .map((item: KungfuApi.Position): KungfuApi.InstrumentForSub => {
        const uidKey = hashInstrumentUKey(item.instrument_id, item.exchange_id);
        return {
          uidKey,
          exchangeId: item.exchange_id,
          instrumentId: item.instrument_id,
          instrumentType: item.instrument_type,
          instrumentName: '',
          ukey: uidKey,
          id: uidKey,
        };
      });
  };

  const subscribeInstrumentsByCurPosAndProcessIds = (
    instrumentsForSub: KungfuApi.InstrumentForSub[],
    filterByCached = true,
  ) => {
    const instrumentsForSubResolved = instrumentsForSub.filter((item) => {
      if (!filterByCached) return true;
      if (filterByCached && !curSubscribedInstruments[item.uidKey]) {
        return true;
      }
      return false;
    });

    if (!instrumentsForSubResolved.length) return;

    subscribeAllInstrumentByAppStates(
      processStatusData.value,
      appStates.value,
      mdExtTypeMap.value,
      instrumentsForSubResolved,
    ).then((subscribedSuccessInstruments) => {
      const subscribedInstrumentsMap = (
        subscribedSuccessInstruments as KungfuApi.InstrumentForSub[]
      ).reduce((subscribedInstruments, item) => {
        subscribedInstruments[item.uidKey] = true;
        return subscribedInstruments;
      }, {} as Record<string, boolean>);

      setCurSubscribedInstruments(subscribedInstrumentsMap);
    });
  };

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject
        .pipe(throttleTime(3000))
        .subscribe((watcher: KungfuApi.Watcher) => {
          const instrumentsForSub = customInstrumentsForSubGetter
            ? customInstrumentsForSubGetter(watcher)
            : getCurrentPositionsForSub(watcher);
          subscribeInstrumentsByCurPosAndProcessIds(instrumentsForSub);
        });

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  watch(appStates, (newAppStates, oldAppStates) => {
    Object.keys(newAppStates || {}).forEach((processId: string) => {
      const newState = newAppStates[processId];
      const oldState = oldAppStates[processId];

      if (
        isBrokerStateReady(newState) &&
        !isBrokerStateReady(oldState) &&
        processStatusData.value[processId] === 'online' &&
        processId.includes('md_')
      ) {
        const instrumentsForSub = customInstrumentsForSubGetter
          ? customInstrumentsForSubGetter(window.watcher)
          : [
              ...subscribedInstrumentsByLocal.value.map((instrument) => ({
                ...instrument,
                uidKey: instrument.ukey,
              })),
              ...getCurrentPositionsForSub(window.watcher),
            ];

        subscribeInstrumentsByCurPosAndProcessIds(instrumentsForSub, false);
      }
    });
  });
};

export const getInstrumentByInstrumentPair = (
  instrumentPair: {
    instrument_id: string;
    instrument_type: InstrumentTypeEnum;
    exchange_id: string;
  },
  instruments: KungfuApi.InstrumentResolved[],
): KungfuApi.InstrumentResolved => {
  const { instrument_id, instrument_type, exchange_id } = instrumentPair;
  const ukey = hashInstrumentUKey(instrument_id, exchange_id);
  const targetInstrumnet: KungfuApi.InstrumentResolved | null =
    findTargetFromArray<KungfuApi.InstrumentResolved>(
      instruments,
      'ukey',
      ukey,
    );
  const instrumentName = targetInstrumnet?.instrumentName || '';
  const instrumentType = targetInstrumnet?.instrumentType;
  return {
    exchangeId: exchange_id,
    instrumentId: instrument_id,
    instrumentType: instrumentType || instrument_type,
    instrumentName,
    ukey,
    id: `${instrument_id}_${instrumentName}_${exchange_id}`.toLowerCase(),
  };
};

export const useQuote = (): {
  quotes: Ref<Record<string, KungfuApi.Quote>>;
  getQuoteByInstrument(
    instrument: KungfuApi.InstrumentResolved | undefined,
  ): KungfuApi.Quote | null;
  getLastPricePercent(
    instrument: KungfuApi.InstrumentResolved | undefined,
  ): string;
  getPreClosePrice(
    instrument: KungfuApi.InstrumentResolved | undefined,
  ): string;
  isInstrumentUpLimit: (instrument: KungfuApi.InstrumentResolved) => boolean;
  isInstrumentLowLimit: (instrument: KungfuApi.InstrumentResolved) => boolean;
  isInstrumentSuspension: (instrument: KungfuApi.InstrumentResolved) => boolean;
} => {
  const quotes = ref<Record<string, KungfuApi.Quote>>({});
  const app = getCurrentInstance();

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject.subscribe(
        (watcher: KungfuApi.Watcher) => {
          quotes.value = toRaw({ ...watcher.ledger.Quote });
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  const getQuoteByInstrument = (
    instrument: KungfuApi.InstrumentResolved | undefined,
  ): KungfuApi.Quote | null => {
    if (!instrument) {
      return null;
    }

    const { ukey } = instrument;
    const quote = quotes.value[ukey] as KungfuApi.Quote | undefined;
    return quote || null;
  };

  const getLastPricePercent = (
    instrument: KungfuApi.InstrumentResolved,
  ): string => {
    const quote = getQuoteByInstrument(instrument);

    if (!quote) {
      return '--';
    }

    const { open_price, last_price } = quote;
    if (!open_price || !last_price) {
      return '--';
    }

    const percent = (last_price - open_price) / open_price;
    if (percent === Infinity) {
      return '--';
    }

    return Number(percent * 100).toFixed(2) + '%';
  };

  const getPreClosePrice = (
    instrument: KungfuApi.InstrumentResolved,
  ): string => {
    const quote = getQuoteByInstrument(instrument);

    if (!quote) {
      return '--';
    }

    const { pre_close_price } = quote;
    return pre_close_price.toFixed(2);
  };

  const isInstrumentUpLimit = (instrument: KungfuApi.InstrumentResolved) => {
    const quote = getQuoteByInstrument(instrument);

    if (!quote) {
      return false;
    }

    const { last_price, upper_limit_price, pre_close_price } = quote;

    if (!last_price || !upper_limit_price || !pre_close_price) return false;

    return (
      last_price >= upper_limit_price || last_price >= pre_close_price * 1.1
    );
  };

  const isInstrumentLowLimit = (instrument: KungfuApi.InstrumentResolved) => {
    const quote = getQuoteByInstrument(instrument);

    if (!quote) {
      return false;
    }

    const { last_price, lower_limit_price, pre_close_price } = quote;

    if (!last_price || !lower_limit_price || !pre_close_price) return false;

    return (
      last_price <= lower_limit_price || last_price <= pre_close_price * 0.9
    );
  };

  const isInstrumentSuspension = (instrument: KungfuApi.InstrumentResolved) => {
    const quote = getQuoteByInstrument(instrument);

    if (!quote) {
      return false;
    }

    const { exchangeId } = instrument;
    const { trading_phase_code } = quote;

    if (!trading_phase_code) return false;

    switch (exchangeId) {
      case 'SSE':
        return trading_phase_code[0] === 'P' || trading_phase_code[0] === 'N';
      case 'SZE':
        return (
          trading_phase_code[0] === 'H' ||
          trading_phase_code[0] === 'V' ||
          trading_phase_code[1] === '1'
        );
      default:
        return false;
    }
  };

  return {
    quotes,
    getQuoteByInstrument,
    getLastPricePercent,
    getPreClosePrice,
    isInstrumentUpLimit,
    isInstrumentLowLimit,
    isInstrumentSuspension,
  };
};

export const useDealInstruments = (): void => {
  const app = getCurrentInstance();
  const dealInstrumentController = ref<boolean>(false);
  const existedInstrumentsLength = ref<number>(0);
  const dealedInstrumentsLength = ref<number>(0);

  onMounted(() => {
    if (app?.proxy) {
      dealInstrumentController.value = true;
      window.workers.dealInstruments.postMessage({
        tag: 'req_instruments',
      });

      const subscription = app.proxy.$tradingDataSubject
        .pipe(throttleTime(5000))
        .subscribe((watcher: KungfuApi.Watcher) => {
          const instruments = watcher.ledger.Instrument.list();
          const instrumentsLength = instruments.length;
          if (!instruments || !instrumentsLength) {
            return;
          }

          if (
            !dealInstrumentController.value &&
            instrumentsLength > dealedInstrumentsLength.value
          ) {
            dealInstrumentController.value = true;
            dealedInstrumentsLength.value = instrumentsLength;
            instruments.forEach((item: KungfuApi.Instrument) => {
              item.ukey = item.uid_key;
            });
            window.workers.dealInstruments.postMessage({
              tag: 'req_dealInstruments',
              instruments: instruments,
            });
          }
        });

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  window.workers.dealInstruments.onmessage = (event: {
    data: {
      tag: string;
      instruments: Record<string, KungfuApi.InstrumentResolved>;
    };
  }) => {
    const { instruments } = event.data || {};

    const instrumentsValue = Object.values(instruments);
    console.log('DealInstruments onmessage', instrumentsValue.length);
    dealInstrumentController.value = false;
    if (instrumentsValue.length) {
      existedInstrumentsLength.value = instrumentsValue.length || 0; //refresh old instruments
      useGlobalStore().setInstruments(instrumentsValue);
      useGlobalStore().setInstrumentsMap(instruments);
    }
  };
};

export const useActiveInstruments = () => {
  const { instrumentsMap } = useGlobalStore();

  const getInstrumentByIds = (
    instrumentId: string,
    exchangeId: string,
    forceConvert = false,
  ) => {
    const ukey = hashInstrumentUKey(instrumentId, exchangeId);
    const instrumentResolved = instrumentsMap[ukey];

    if (instrumentResolved) {
      return instrumentResolved;
    } else {
      return forceConvert
        ? {
            instrumentId: instrumentId,
            exchangeId: exchangeId,
            instrumentType: window.watcher.getInstrumentType(
              exchangeId,
              instrumentId,
            ),
            ukey,
            instrumentName: '',
            id: `${instrumentId}_${''}_${exchangeId}`.toLowerCase(),
          }
        : null;
    }
  };

  return {
    getInstrumentByIds,
  };
};

export const useProcessStatusDetailData = (): {
  processStatusData: Ref<Pm2ProcessStatusData>;
  processStatusDetailData: Ref<Pm2ProcessStatusDetailData>;
  appStates: Ref<Record<string, BrokerStateStatusTypes>>;
  getProcessStatusName(
    kfConfig: KungfuApi.KfLocation,
  ): ProcessStatusTypes | undefined;
  getStrategyStatusName(
    kfConfig: KungfuApi.KfLocation,
  ): ProcessStatusTypes | undefined;
} => {
  const allProcessStatusData = reactive<{
    processStatusData: Pm2ProcessStatusData;
    processStatusDetailData: Pm2ProcessStatusDetailData;
    appStates: Record<string, BrokerStateStatusTypes>;
    strategyStates: Record<string, KungfuApi.StrategyStateData>;
  }>({
    processStatusData: {},
    processStatusDetailData: {},
    appStates: {},
    strategyStates: {},
  });

  onMounted(() => {
    const {
      processStatusData,
      processStatusWithDetail,
      appStates,
      strategyStates,
    } = storeToRefs(useGlobalStore());
    allProcessStatusData.processStatusData =
      processStatusData as unknown as Pm2ProcessStatusData;
    allProcessStatusData.processStatusDetailData =
      processStatusWithDetail as unknown as Pm2ProcessStatusDetailData;
    allProcessStatusData.appStates = appStates as unknown as Record<
      string,
      BrokerStateStatusTypes
    >;
    allProcessStatusData.strategyStates = strategyStates as unknown as Record<
      string,
      KungfuApi.StrategyStateData
    >;
  });

  const getProcessStatusName = (kfConfig: KungfuApi.KfLocation) => {
    return getAppStateStatusName(
      kfConfig,
      allProcessStatusData.processStatusData,
      allProcessStatusData.appStates,
    );
  };

  const getStrategyStatusName = (kfConfig: KungfuApi.KfLocation) => {
    return getStrategyStateStatusName(
      kfConfig,
      allProcessStatusData.processStatusData,
      allProcessStatusData.strategyStates,
    );
  };

  const { processStatusData, processStatusDetailData, appStates } =
    toRefs(allProcessStatusData);

  return {
    processStatusData,
    processStatusDetailData,
    appStates,
    getProcessStatusName,
    getStrategyStatusName,
  };
};

export const useExtConfigsRelated = (): {
  extConfigs: Ref<KungfuApi.KfExtConfigs>;
  uiExtConfigs: Ref<KungfuApi.KfUIExtConfigs>;
  tdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
  mdExtTypeMap: ComputedRef<Record<string, InstrumentTypes>>;
  strategyExtTypeMap: ComputedRef<Record<string, StrategyExtTypes>>;
} => {
  const { extConfigs, uiExtConfigs } = storeToRefs(useGlobalStore());
  const tdExtTypeMap = computed(
    () =>
      buildExtTypeMap(extConfigs.value, 'td') as Record<
        string,
        InstrumentTypes
      >,
  );
  const mdExtTypeMap = computed(
    () =>
      buildExtTypeMap(extConfigs.value, 'md') as Record<
        string,
        InstrumentTypes
      >,
  );

  const strategyExtTypeMap = computed(
    () =>
      buildExtTypeMap(extConfigs.value, 'strategy') as Record<
        string,
        StrategyExtTypes
      >,
  );

  return {
    extConfigs,
    uiExtConfigs,
    tdExtTypeMap,
    mdExtTypeMap,
    strategyExtTypeMap,
  };
};

export const useCurrentGlobalKfLocation = (
  watcher: KungfuApi.Watcher | null,
): {
  currentGlobalKfLocation: Ref<
    KungfuApi.KfLocation | KungfuApi.KfConfig | null
  >;
  currentCategoryData: ComputedRef<KungfuApi.KfTradeValueCommonData | null>;
  currentUID: ComputedRef<string>;
  setCurrentGlobalKfLocation(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): void;
  resetCurrentGlobalKfLocation(): void;
  dealRowClassName(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): string;
  customRow(
    kfConfig:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): {
    onClick(): void;
  };
  getCurrentGlobalKfLocationId(
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ): string;
} => {
  const { currentGlobalKfLocation } = storeToRefs(useGlobalStore());

  const setCurrentGlobalKfLocation = (
    kfLocation:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ) => {
    useGlobalStore().setCurrentGlobalKfLocation(kfLocation);
  };

  const resetCurrentGlobalKfLocation = () => {
    useGlobalStore().setCurrentGlobalKfLocation(null);
    useGlobalStore().setDefaultCurrentGlobalKfLocation();
  };

  const dealRowClassName = (
    record:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ): string => {
    if (currentGlobalKfLocation.value === null) return '';

    if (
      getIdByKfLocation(record) ===
      getIdByKfLocation(currentGlobalKfLocation.value)
    ) {
      return 'current-global-kfLocation';
    }

    return '';
  };

  const customRow = (
    record:
      | KungfuApi.KfLocation
      | KungfuApi.KfConfig
      | KungfuApi.KfExtraLocation,
  ) => {
    return {
      onClick: () => {
        setCurrentGlobalKfLocation(record);
      },
    };
  };

  const currentCategoryData = computed(() => {
    if (!currentGlobalKfLocation.value) {
      return null;
    }

    const extraCategory: Record<string, KungfuApi.KfTradeValueCommonData> =
      globalThis.HookKeeper.getHooks().dealTradingData.getCategoryMap();
    return dealCategory(currentGlobalKfLocation.value?.category, extraCategory);
  });

  const currentUID = computed(() => {
    if (!watcher) {
      return '';
    }

    if (!currentGlobalKfLocation.value) {
      return '';
    }

    return watcher.getLocationUID(currentGlobalKfLocation.value);
  });

  const getCurrentGlobalKfLocationId = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig | null,
  ): string => {
    if (!kfConfig) {
      return '';
    }

    return getIdByKfLocation(kfConfig) || '';
  };

  return {
    currentGlobalKfLocation,
    currentCategoryData,
    currentUID,
    setCurrentGlobalKfLocation,
    resetCurrentGlobalKfLocation,
    dealRowClassName,
    customRow,
    getCurrentGlobalKfLocationId,
  };
};

export const useAllKfConfigData = (): Record<
  KfCategoryTypes,
  KungfuApi.KfLocation[]
> => {
  const allKfConfigData: Record<KfCategoryTypes, KungfuApi.KfLocation[]> =
    reactive({
      system: ref<(KungfuApi.KfConfig | KungfuApi.KfExtraLocation)[]>([
        ...(process.env.NODE_ENV === 'development'
          ? [
              {
                location_uid: 0,
                category: 'system',
                group: 'service',
                name: 'archive',
                mode: 'live',
                value: '',
              },
            ]
          : []),
        {
          location_uid: 0,
          category: 'system',
          group: 'master',
          name: 'master',
          mode: 'live',
          value: '',
        },
        {
          location_uid: 0,
          category: 'system',
          group: 'service',
          name: 'cached',
          mode: 'live',
          value: '',
        },
        {
          location_uid: 0,
          category: 'system',
          group: 'service',
          name: 'ledger',
          mode: 'live',
          value: '',
        },
      ]),

      daemon: [],
      md: [],
      td: [],
      strategy: [],
    });

  onMounted(() => {
    const { mdList, tdList, strategyList } = storeToRefs(useGlobalStore());

    allKfConfigData.md = mdList as unknown as KungfuApi.KfConfig[];
    allKfConfigData.td = tdList as unknown as KungfuApi.KfConfig[];
    allKfConfigData.strategy = strategyList as unknown as KungfuApi.KfConfig[];

    getAvailDaemonList().then((daemonList) => {
      allKfConfigData.daemon = daemonList;
    });
  });

  return allKfConfigData;
};

export const useTdGroups = (): Ref<KungfuApi.KfExtraLocation[]> => {
  const { tdGroupList } = storeToRefs(useGlobalStore());
  return tdGroupList;
};

export const useAssets = (): {
  assets: Ref<Record<string, KungfuApi.Asset>>;
  getAssetsByKfConfig(
    kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): KungfuApi.Asset;
  getAssetsByTdGroup(tdGroup: KungfuApi.KfExtraLocation): KungfuApi.Asset;
} => {
  const { assets } = storeToRefs(useGlobalStore());

  const getAssetsByKfConfig = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): KungfuApi.Asset => {
    const processId = getProcessIdByKfLocation(kfConfig);
    return assets.value[processId] || ({} as KungfuApi.Asset);
  };

  const getAssetsByTdGroup = (
    tdGroup: KungfuApi.KfExtraLocation,
  ): KungfuApi.Asset => {
    const children = (tdGroup.children || []) as KungfuApi.KfConfig[];
    const assetsList = children
      .map((item) => getAssetsByKfConfig(item))
      .filter((item) => Object.keys(item).length);

    return assetsList.reduce((allAssets, asset) => {
      return {
        ...allAssets,
        unrealized_pnl: (allAssets.unrealized_pnl || 0) + asset.unrealized_pnl,
        market_value: (allAssets.market_value || 0) + asset.market_value,
        margin: (allAssets.margin || 0) + asset.margin,
        avail: (allAssets.avail || 0) + asset.avail,
      };
    }, {} as KungfuApi.Asset);
  };

  return {
    assets,
    getAssetsByKfConfig,
    getAssetsByTdGroup,
  };
};

export const useAssetMargins = () => {
  const app = getCurrentInstance();
  const assetMagins = ref<Record<string, KungfuApi.AssetMargin>>({});

  const getAssetMarginsByKfConfig = (
    kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
  ): KungfuApi.AssetMargin => {
    const processId = getProcessIdByKfLocation(kfConfig);
    return assetMagins.value[processId] || ({} as KungfuApi.AssetMargin);
  };

  const getAssetMarginsByTdGroup = (
    tdGroup: KungfuApi.KfExtraLocation,
  ): KungfuApi.AssetMargin => {
    const children = (tdGroup.children || []) as KungfuApi.KfConfig[];
    const assetMarginsList = children
      .map((item) => getAssetMarginsByKfConfig(item))
      .filter((item) => Object.keys(item).length);

    return assetMarginsList.reduce((allAssetMargins, assetMagin) => {
      return {
        ...allAssetMargins,
        margin: (allAssetMargins.margin || 0) + assetMagin.margin,
        avail_margin:
          (allAssetMargins.avail_margin || 0) + assetMagin.avail_margin,
        market_value: (allAssetMargins.cash_debt || 0) + assetMagin.cash_debt,
        avail: (allAssetMargins.total_asset || 0) + assetMagin.total_asset,
      };
    }, {} as KungfuApi.AssetMargin);
  };

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject.subscribe(
        (watcher: KungfuApi.Watcher) => {
          assetMagins.value = dealAssetsByHolderUID<KungfuApi.AssetMargin>(
            watcher,
            watcher.ledger.AssetMargin,
          );
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    assetMagins,
    getAssetMarginsByKfConfig,
    getAssetMarginsByTdGroup,
  };
};

export const playSound = (type: 'ding' | 'warn' = 'ding'): void => {
  const soundPath = path.join(
    `${path.join(KUNGFU_RESOURCES_DIR, `music/${type}.mp3`)}`,
  );
  const { globalSetting } = storeToRefs(useGlobalStore());
  if (globalSetting.value?.trade?.sound) {
    sound.play(soundPath);
  }
};

export const useCurrentPositionList = () => {
  const app = getCurrentInstance();
  type PosStat = Record<string, KungfuApi.Position>;

  const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(
    window.watcher,
  );
  const currentPositionList = ref<KungfuApi.PositionResolved[]>([]);
  const allPositionMap = ref<PosStat>({});

  function buildPrositionMapKey(
    exchangeId: string,
    instrumentId: string,
    direction: DirectionEnum,
  ) {
    return `${exchangeId}_${instrumentId}_${direction}`;
  }

  function buildGlobalPositions(positions: KungfuApi.Position[]): PosStat {
    return positions.reduce((posStat, pos) => {
      const id = buildPrositionMapKey(
        pos.exchange_id,
        pos.instrument_id,
        pos.direction,
      );
      if (!posStat[id]) {
        posStat[id] = pos;
      } else {
        const prePosStat = posStat[id];
        const { avg_open_price, volume, yesterday_volume, unrealized_pnl } =
          prePosStat;
        posStat[id] = {
          ...prePosStat,
          uid_key: pos.uid_key,
          yesterday_volume: yesterday_volume + pos.yesterday_volume,
          volume: volume + pos.volume,

          avg_open_price:
            (avg_open_price * Number(volume) +
              pos.avg_open_price * Number(pos.volume)) /
            (Number(pos.volume) + Number(pos.volume)),
          unrealized_pnl: unrealized_pnl + pos.unrealized_pnl,
        };
      }
      return posStat;
    }, {} as PosStat);
  }

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject.subscribe(
        (watcher: KungfuApi.Watcher) => {
          const allPositions = watcher.ledger.Position.nofilter(
            'volume',
            BigInt(0),
          )
            .filter('ledger_category', LedgerCategoryEnum.td)
            .list();

          allPositionMap.value = toRaw(buildGlobalPositions(allPositions));

          if (currentGlobalKfLocation.value === null) {
            return;
          }

          const currentPositions =
            globalThis.HookKeeper.getHooks().dealTradingData.trigger(
              window.watcher,
              currentGlobalKfLocation.value,
              watcher.ledger.Position,
              'position',
            ) as KungfuApi.Position[];

          currentPositionList.value = toRaw(
            currentPositions
              .reverse()
              .map((item) => dealPosition(watcher, item)),
          );
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    allPositionMap,
    buildPrositionMapKey,
    currentPositionList,
  };
};

export const useMakeOrderInfo = (
  formState: Ref<Record<string, KungfuApi.KfConfigValue>>,
) => {
  const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(
    window.watcher,
  );
  const { currentPositionList } = useCurrentPositionList();
  const { getAssetsByKfConfig } = useAssets();

  const instrumentResolved = computed(() => {
    const { instrument } = formState.value;
    return instrument
      ? transformSearchInstrumentResultToInstrument(instrument)
      : null;
  });

  const isCurrentCategoryIsTd = computed(
    () => currentGlobalKfLocation.value?.category === 'td',
  );

  const isAccountOrInstrumentConfirmed = computed(() => {
    if (formState.value?.side === SideEnum.Buy) {
      return isCurrentCategoryIsTd.value ? true : !!formState.value.account_id;
    } else if (formState.value.side === SideEnum.Sell) {
      return isCurrentCategoryIsTd.value
        ? !!formState.value.instrument
        : formState.value.account_id && formState.value.instrument;
    }
    return false;
  });

  const showAmountOrPosition = computed(() => {
    const { offset } = formState.value;

    return offset === OffsetEnum.Open ? 'amount' : 'position';
  });

  const currentAccountLocation = computed(() => {
    if (currentGlobalKfLocation.value && isCurrentCategoryIsTd.value) {
      return currentGlobalKfLocation.value;
    } else if (formState.value.account_id) {
      const { source, id } = formState.value.account_id.parseSourceAccountId();
      return {
        category: 'td',
        group: source,
        name: id,
        mode: 'live',
      } as KungfuApi.KfLocation;
    } else {
      return null;
    }
  });

  const currentFormDirection = computed(() => {
    const { side, offset } = formState.value;

    if (side === SideEnum.Buy) {
      if (offset === OffsetEnum.Open) {
        return DirectionEnum.Long;
      } else {
        return DirectionEnum.Short;
      }
    } else if (side === SideEnum.Sell) {
      if (offset === OffsetEnum.Open) {
        return DirectionEnum.Short;
      } else {
        return DirectionEnum.Long;
      }
    }

    return null;
  });

  const getPositionByInstrumentAndDirection = (
    positionList: KungfuApi.PositionResolved[],
    instrument: KungfuApi.InstrumentResolved | null,
    direction: DirectionEnum,
  ) => {
    if (!currentAccountLocation.value) return null;
    if (!positionList.length || !instrument) return null;

    const { exchangeId, instrumentId, instrumentType } = instrument;
    const targetPositionList: KungfuApi.Position[] = positionList.filter(
      (position) =>
        position.exchange_id === exchangeId &&
        position.instrument_id === instrumentId &&
        position.instrument_type === instrumentType &&
        position.account_id_resolved ===
          getIdByKfLocation(
            currentAccountLocation.value as KungfuApi.KfLocation,
          ),
    );

    if (targetPositionList && targetPositionList.length) {
      const targetPositionWithLongDirection = targetPositionList.filter(
        (item) => item.direction === direction,
      );

      if (targetPositionWithLongDirection.length) {
        return targetPositionWithLongDirection[0];
      }
    }

    return null;
  };

  const currentPositionWithLongDirection = computed(() => {
    return getPositionByInstrumentAndDirection(
      currentPositionList.value,
      instrumentResolved.value,
      DirectionEnum.Long,
    );
  });

  const currentPositionWithShortDirection = computed(() => {
    return getPositionByInstrumentAndDirection(
      currentPositionList.value,
      instrumentResolved.value,
      DirectionEnum.Short,
    );
  });

  const currentPosition = computed(() => {
    if (currentFormDirection.value === DirectionEnum.Long) {
      return currentPositionWithLongDirection.value;
    } else if (currentFormDirection.value === DirectionEnum.Short) {
      return currentPositionWithShortDirection.value;
    }

    return null;
  });

  const currentAvailMoney = computed(() => {
    if (!currentAccountLocation.value) return '--';

    const avail = getAssetsByKfConfig(currentAccountLocation.value).avail;

    return dealKfPrice(avail);
  });

  const currentAvailPosVolume = computed(() => {
    if (!instrumentResolved.value) return '--';

    const { instrumentType, exchangeId } = instrumentResolved.value;
    const { offset } = formState.value;

    if (currentPosition.value) {
      const { yesterday_volume, volume, frozen_total } = currentPosition.value;
      const today_volume = volume - yesterday_volume;
      const frozen_today = frozen_total - frozen_total;
      const closable_yesterday = yesterday_volume - frozen_total;
      const closable_today = today_volume - frozen_today;
      const closable_total = volume - frozen_total;

      if (isShotable(instrumentType) || isT0(instrumentType, exchangeId)) {
        if (offset === OffsetEnum.CloseYest) {
          return dealKfNumber(closable_yesterday) + '';
        } else if (offset === OffsetEnum.CloseToday) {
          return dealKfNumber(closable_today) + '';
        } else {
          return dealKfNumber(closable_total) + '';
        }
      } else {
        return dealKfNumber(closable_yesterday) + '';
      }
    }

    return '0';
  });

  function dealTradeAmount(preNumber: number | null) {
    return !Number(preNumber) ? '--' : dealKfPrice(preNumber);
  }

  const currentPrice = computed(() => {
    const { price_type, limit_price } = formState.value;

    if (price_type === PriceTypeEnum.Limit) {
      return limit_price as number;
    } else if (price_type === PriceTypeEnum.Market) {
      return currentPosition.value?.last_price ?? null;
    }

    return null;
  });

  const currentTradeAmount = computed(() => {
    const { volume } = formState.value;

    if (instrumentResolved.value) {
      const instrumentForAccounting: KungfuApi.InstrumentForAccounting = {
        ...instrumentResolved.value,
        price: currentPrice.value ?? 0,
        volume,
        direction: currentFormDirection.value || DirectionEnum.Long,
      };
      if (instrumentResolved.value.instrumentType in TradeAccountingUsageMap) {
        return dealTradeAmount(
          TradeAccountingUsageMap[
            instrumentResolved.value.instrumentType as InstrumentTypeEnum
          ].getTradeAmount(window.watcher, instrumentForAccounting),
        );
      }
    }

    return dealTradeAmount((currentPrice.value ?? 0) * volume);
  });

  const currentResidueMoney = computed(() => {
    const { offset } = formState.value;
    if (currentAvailMoney.value !== '--') {
      if (currentTradeAmount.value !== '--') {
        if (offset === OffsetEnum.Open) {
          return dealKfPrice(
            Number(currentAvailMoney.value) - Number(currentTradeAmount.value),
          );
        } else {
          return dealKfPrice(
            Number(currentAvailMoney.value) + Number(currentTradeAmount.value),
          );
        }
      } else {
        return currentAvailMoney.value;
      }
    } else {
      return '--';
    }
  });

  const currentResiduePosVolume = computed(() => {
    const { volume, offset } = formState.value;
    if (currentAvailPosVolume.value !== '--') {
      if (volume && volume > 0) {
        if (offset === OffsetEnum.Open) {
          return dealKfNumber(
            Number(currentAvailPosVolume.value) + Number(volume),
          );
        } else {
          return dealKfNumber(
            Number(currentAvailPosVolume.value) - Number(volume),
          );
        }
      } else {
        return currentAvailPosVolume.value;
      }
    } else {
      return '--';
    }
  });

  return {
    showAmountOrPosition,
    isAccountOrInstrumentConfirmed,
    instrumentResolved,
    currentPosition,
    currentPositionWithLongDirection,
    currentPositionWithShortDirection,
    currentAvailMoney,
    currentAvailPosVolume,
    currentPrice,
    currentTradeAmount,
    currentResidueMoney,
    currentResiduePosVolume,
  };
};

export const useTradeLimit = () => {
  const store = useGlobalStore();
  const app = getCurrentInstance();
  const { globalSetting } = storeToRefs(store);
  type LimitRuleType = {
    instrument: string;
    value: Record<OrderInputKeyEnum, number>;
  };
  type LimitRulesMapType = Record<string, LimitRuleType>;
  const limitRulesMapRef = ref<LimitRulesMapType>({});

  const setLimitRulesMap = () => {
    limitRulesMapRef.value = (
      (globalSetting?.value?.trade?.limit || []) as KungfuApi.TradeLimitItem[]
    ).reduce((map, item) => {
      const { instrument, orderInputKey, limitValue } = item;
      if (map[instrument]?.value) {
        const oldValue = map[instrument].value[orderInputKey]
          ? map[instrument].value[orderInputKey]
          : Infinity;
        map[instrument].value[orderInputKey] = Math.min(limitValue, oldValue);
      } else {
        map[instrument] = {
          instrument,
          value: {
            [orderInputKey]: limitValue,
          } as LimitRuleType['value'],
        };
      }

      return map;
    }, {} as LimitRulesMapType);
  };

  const createValidatorByLimitRule = (
    limitRule: LimitRuleType,
    orderInputKey: OrderInputKeyEnum,
  ) => {
    const orderInputKeyName = OrderInputKeySetting[orderInputKey].name;
    return function (_rule: RuleObject, value: string | number) {
      if (Number.isNaN(+value))
        return Promise.reject(new Error(t('blockTradeConfig.only_number')));
      value = Number(value);

      if (value < 0) {
        return Promise.reject(new Error(t('validate.no_negative_number')));
      }

      const limitValue = limitRule?.value?.[orderInputKey];

      if (limitValue !== undefined) {
        if (limitValue < value) {
          return Promise.reject(
            new Error(
              t('tradeConfig.greater_than_limit_value', {
                key: orderInputKeyName,
                value: limitValue,
              }),
            ),
          );
        }
      }

      return Promise.resolve();
    };
  };

  const getValidatorByOrderInputKey = (
    orderInputKey: OrderInputKeyEnum,
    instrument: string,
  ) => {
    const currentLimitRule = limitRulesMapRef.value[instrument];

    return createValidatorByLimitRule(currentLimitRule, orderInputKey);
  };

  onMounted(() => {
    setLimitRulesMap();

    if (app?.proxy) {
      const subscription = app.proxy.$globalBus.subscribe(
        (data: KfEvent.KfBusEvent) => {
          if (data.tag === 'saved:globalSetting') {
            setLimitRulesMap();
          }
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    getValidatorByOrderInputKey,
  };
};

export const useMakeOrderSubscribe = (
  formState: Ref<Record<string, KungfuApi.KfConfigValue>>,
) => {
  const app = getCurrentInstance();
  let lastTriggerTag: 'makeOrder' | 'orderBookUpdate' | '' = '';
  let lastVolume = 0;
  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$globalBus.subscribe(
        (data: KfEvent.KfBusEvent) => {
          if (data.tag === 'makeOrder') {
            const { offset, side, volume, price, instrumentType, accountId } = (
              data as KfEvent.TriggerMakeOrder
            ).orderInput;

            const instrumentValue = buildInstrumentSelectOptionValue(
              (data as KfEvent.TriggerMakeOrder).orderInput,
            );

            formState.value.instrument = instrumentValue;
            formState.value.offset = +offset;
            formState.value.side = +side;
            formState.value.volume = +Number(volume).toFixed(0);
            formState.value.limit_price = +Number(price).toFixed(4);
            formState.value.instrument_type = +instrumentType;

            if (accountId) {
              formState.value.account_id = accountId;
            }
            lastTriggerTag = 'makeOrder';
            lastVolume = formState.value.volume;
          }

          if (data.tag === 'orderBookUpdate') {
            const { side, price, volume, instrumentType } = (
              data as KfEvent.TriggerOrderBookUpdate
            ).orderInput;

            const instrumentValue = buildInstrumentSelectOptionValue(
              (data as KfEvent.TriggerOrderBookUpdate).orderInput,
            );

            if (!formState.value.instrument) {
              formState.value.instrument = instrumentValue;
              formState.value.instrument_type = +instrumentType;
            }

            if (!!price && !Number.isNaN(price) && +price !== 0) {
              formState.value.limit_price = +Number(price).toFixed(4);
            }

            const shouldUpdateVolume =
              (lastTriggerTag === 'orderBookUpdate' &&
                lastVolume === formState.value.volume) ||
              !formState.value.volume;
            const isNewVolumeValuable =
              !!volume &&
              !Number.isNaN(Number(volume)) &&
              BigInt(volume) !== BigInt(0);

            if (shouldUpdateVolume && isNewVolumeValuable) {
              formState.value.volume = +Number(volume).toFixed(0);
              lastVolume = formState.value.volume;
            }

            formState.value.side = +side;
            if (shouldUpdateVolume) {
              lastTriggerTag = 'orderBookUpdate';
            } else {
              lastVolume = 0;
            }
          }
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });
};

export const useBasket = () => {
  const app = getCurrentInstance();
  const store = useGlobalStore();

  const basketList = ref<KungfuApi.Basket[]>([]);
  const basketInstrumentList = ref<KungfuApi.BasketInstrument[]>([]);

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject.subscribe(
        (watcher: KungfuApi.Watcher) => {
          store.setBasketList();
          store.setBasketInstrumentList();

          // const basketInWatcher = watcher.ledger.Basket.sort('id')
          // const basketInstrumentInWatcher = watcher.ledger.BasketInstrument.list()
          watcher;
          basketList.value = store.basketList;
          basketInstrumentList.value = store.basketInstrumentList;
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  function buildBasketOptionLabel(basket: KungfuApi.Basket) {
    return `${basket.name} ${BasketVolumeType[basket.volume_type].name}`;
  }

  function buildBasketOptionValue(basket: KungfuApi.Basket) {
    return `${basket.id}_${basket.name}`;
  }

  function parseBasketOptionValue(val: string): KungfuApi.Basket | null {
    const res = val.split('_');
    if (res.length !== 4) return null;
    const [id, name, volume_type, total_amount] = res;

    return {
      id: Number(id),
      name,
      volume_type: Number(volume_type),
      total_amount: BigInt(total_amount),
    };
  }

  return {
    basketList,
    basketInstrumentList,
    buildBasketOptionLabel,
    buildBasketOptionValue,
    parseBasketOptionValue,
  };
};
