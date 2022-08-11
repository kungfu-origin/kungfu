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
  LedgerCategoryEnum,
  OffsetEnum,
  PriceTypeEnum,
  ProcessStatusTypes,
  SideEnum,
  StrategyExtTypes,
  OrderInputKeyEnum,
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
  removeNoDefaultStrategyFolders,
  getStrategyStateStatusName,
  isBrokerStateReady,
  dealKfNumber,
  dealKfPrice,
  transformSearchInstrumentResultToInstrument,
  booleanProcessEnv,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { writeCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import {
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetailData,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { Modal } from 'ant-design-vue';
import path from 'path';
import { Proc } from 'pm2';
import {
  ComponentInternalInstance,
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
  ShotableInstrumentTypes,
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

const { t } = VueI18n.global;
const { success, error } = messagePrompt();

export const handleSwitchProcessStatus = (
  checked: boolean,
  mouseEvent: MouseEvent,
  kfLocation: KungfuApi.KfLocation,
): Promise<void | Proc> => {
  mouseEvent.stopPropagation();
  return switchKfLocation(window.watcher, kfLocation, checked)
    .then(() => {
      success();
    })
    .catch((err: Error) => {
      error(err.message || t('operation_failed'));
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
      const { tradingData } = await getKungfuHistoryData(
        date,
        dateType,
        tradingDataType,
      );
      const orders = tradingData.Order.sort('update_time');
      const trades = tradingData.Trade.sort('trade_time');
      const orderStat = tradingData.OrderStat.sort('insert_time');
      const positions = tradingData.Trade.list();

      const { filePaths } = await dialog.showOpenDialog({
        properties: ['openDirectory'],
      });

      if (!filePaths) {
        return;
      }

      const targetFolder = filePaths[0];

      const ordersFilename = path.join(targetFolder, `orders-${dateResolved}`);
      const tradesFilename = path.join(targetFolder, `trades-${dateResolved}`);
      const orderStatFilename = path.join(
        targetFolder,
        `orderStats-${dateResolved}`,
      );
      const posFilename = path.join(targetFolder, `pos-${dateResolved}`);

      return Promise.all([
        writeCSV(ordersFilename, orders, dealTradingDataItemResolved()),
        writeCSV(tradesFilename, trades, dealTradingDataItemResolved()),
        writeCSV(
          orderStatFilename,
          orderStat,
          dealTradingDataItemResolved(true),
        ),
        writeCSV(posFilename, positions, dealTradingDataItemResolved()),
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
    const { tradingData } = await getKungfuHistoryData(
      date,
      dateType,
      tradingDataType,
      currentKfLocation,
    );
    exportDataLoading.value = false;

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
): Promise<void> => {
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
  subscribedInstruments: Ref<KungfuApi.InstrumentResolved[]>;
  subscribeAllInstrumentByMdProcessId(
    processId: string,
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ): void;
  subscribeAllInstrumentByAppStates(
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ): void;

  searchInstrumentResult: Ref<string | undefined>;
  searchInstrumnetOptions: Ref<{ value: string; label: string }[]>;
  updateSearchInstrumnetOptions: (
    type: 'instrument' | 'instruments',
    value: string | string[],
  ) => Promise<{ value: string; label: string }[]>;
  handleSearchInstrument: (
    value: string,
  ) => Promise<{ value: string; label: string }[]>;
  handleConfirmSearchInstrumentResult: (
    value: string,
    callback?: (value: string) => void,
  ) => void;
} => {
  const { instruments, subscribedInstruments } = storeToRefs(useGlobalStore());

  const subscribeAllInstrumentByMdProcessId = (
    processId: string,
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ): void => {
    if (isBrokerStateReady(appStates[processId])) {
      if (processStatus[processId] === 'online') {
        if (processId.indexOf('md_') === 0) {
          const mdLocation = getMdTdKfLocationByProcessId(processId);
          if (mdLocation && mdLocation.category === 'md') {
            const sourceId = mdLocation.group;
            const sourceType = mdExtTypeMap[sourceId];
            const ableSubscribedInstrumentTypes =
              AbleSubscribeInstrumentTypesBySourceType[sourceType] || [];

            instrumentsForSubscribe.forEach((item) => {
              if (
                ableSubscribedInstrumentTypes.includes(+item.instrumentType)
              ) {
                kfRequestMarketData(
                  window.watcher,
                  item.exchangeId,
                  item.instrumentId,
                  mdLocation,
                ).catch((err) => console.warn(err.message));
              }
            });
          }
        }
      }
    }
  };

  const subscribeAllInstrumentByAppStates = (
    processStatus: Pm2ProcessStatusData,
    appStates: Record<string, BrokerStateStatusTypes>,
    mdExtTypeMap: Record<string, InstrumentTypes>,
    instrumentsForSubscribe: KungfuApi.InstrumentResolved[],
  ) => {
    Object.keys(appStates || {}).forEach((processId) => {
      subscribeAllInstrumentByMdProcessId(
        processId,
        processStatus,
        appStates,
        mdExtTypeMap,
        instrumentsForSubscribe,
      );
    });
  };

  const searchInstrumentResult = ref<string | undefined>(undefined);
  const searchInstrumnetOptions = ref<{ value: string; label: string }[]>([]);

  const updateSearchInstrumnetOptions = (
    type: 'instrument' | 'instruments',
    value: string | string[],
  ): Promise<{ value: string; label: string }[]> => {
    searchInstrumnetOptions.value = makeSearchOptionFormInstruments(
      type,
      value,
    );
    return Promise.resolve(searchInstrumnetOptions.value);
  };

  const handleSearchInstrument = (
    val: string,
  ): Promise<{ value: string; label: string }[]> => {
    searchInstrumnetOptions.value = instruments.value
      .filter((item) => {
        return !!val && item.id.includes(val);
      })
      .slice(0, 20)
      .map((item) => ({
        value: buildInstrumentSelectOptionValue(item),
        label: buildInstrumentSelectOptionLabel(item),
      }));
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
    subscribedInstruments,
    subscribeAllInstrumentByMdProcessId,
    subscribeAllInstrumentByAppStates,

    searchInstrumentResult,
    searchInstrumnetOptions,
    updateSearchInstrumnetOptions,
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
            }

            if (data.name && data.name === 'extraResourcesLoading') {
              preStartSystemLoadingData.extraResourcesLoading =
                data.status === 'online' ? 'done' : 'loading';
            }

            if (data.name === 'system' && data.status === 'waiting restart') {
              preStartSystemLoadingData.archive = 'loading';
              preStartSystemLoadingData.extraResourcesLoading = 'loading';
              startGetWatcherStatus();
            }
          }

          if (data.tag === 'main') {
            switch (data.name) {
              case 'record-before-quit':
                preQuitSystemLoadingData.record = 'loading';
                preQuitTasks([removeNoDefaultStrategyFolders()]).finally(() => {
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

export const useSubscibeInstrumentAtEntry = (): void => {
  const app = getCurrentInstance();
  const subscribedInstrumentsForPos: Record<string, boolean> = {};

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject
        .pipe(throttleTime(3000))
        .subscribe((watcher: KungfuApi.Watcher) => {
          const bigint0 = BigInt(0);
          const positions = watcher.ledger.Position.filter('ledger_category', 0)
            .nofilter('volume', bigint0)
            .list()
            .map(
              (item: KungfuApi.Position): KungfuApi.InstrumentForSub => ({
                uidKey: item.uid_key,
                exchangeId: item.exchange_id,
                instrumentId: item.instrument_id,
                instrumentType: item.instrument_type,
                mdLocation: watcher.getLocation(item.holder_uid),
              }),
            );

          positions.forEach((item) => {
            if (subscribedInstrumentsForPos[item.uidKey]) {
              return;
            }

            const { group } = item.mdLocation;
            const mdLocationResolved: KungfuApi.KfLocation = {
              category: 'md',
              group,
              name: group,
              mode: 'live',
            };

            kfRequestMarketData(
              watcher,
              item.exchangeId,
              item.instrumentId,
              mdLocationResolved,
            ).catch((err) => console.warn(err.message));
            subscribedInstrumentsForPos[item.uidKey] = true;
          });
        });

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  const { appStates, processStatusData } = useProcessStatusDetailData();
  const { mdExtTypeMap } = useExtConfigsRelated();
  const { subscribedInstruments, subscribeAllInstrumentByMdProcessId } =
    useInstruments();
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
        const positions: KungfuApi.InstrumentResolved[] =
          window.watcher.ledger.Position.nofilter('volume', BigInt(0))
            .filter('ledger_category', LedgerCategoryEnum.td)
            .list()
            .map((item: KungfuApi.Position) => ({
              instrumentId: item.instrument_id,
              instrumentName: '',
              exchangeId: item.exchange_id,
              instrumentType: item.instrument_type,
              ukey: item.uid_key,
              id: item.uid_key,
            }));
        subscribeAllInstrumentByMdProcessId(
          processId,
          processStatusData.value,
          appStates.value,
          mdExtTypeMap.value,
          [...subscribedInstruments.value, ...positions],
        );
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

  return {
    quotes,
    getQuoteByInstrument,
    getLastPricePercent,
    getPreClosePrice,
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
    data: { tag: string; instruments: KungfuApi.InstrumentResolved[] };
  }) => {
    const { instruments } = event.data || {};

    console.log('DealInstruments onmessage', instruments.length);
    dealInstrumentController.value = false;
    if (instruments.length) {
      existedInstrumentsLength.value = instruments.length || 0; //refresh old instruments
      useGlobalStore().setInstruments(instruments);
    }
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

export const playSound = (): void => {
  const soundPath = path.join(
    `${path.join(KUNGFU_RESOURCES_DIR, 'music/ding.mp3')}`,
  );
  const { globalSetting } = storeToRefs(useGlobalStore());
  if (globalSetting.value?.trade?.sound) {
    sound.play(soundPath);
  }
};

export const useCurrentPositionList = (
  app: ComponentInternalInstance | null,
) => {
  const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(
    window.watcher,
  );
  const currentPositionList = ref<KungfuApi.Position[]>([]);

  onMounted(() => {
    if (app?.proxy) {
      const subscription = app.proxy.$tradingDataSubject.subscribe(
        (watcher: KungfuApi.Watcher) => {
          if (currentGlobalKfLocation.value === null) {
            return;
          }

          const positions =
            globalThis.HookKeeper.getHooks().dealTradingData.trigger(
              window.watcher,
              currentGlobalKfLocation.value,
              watcher.ledger.Position,
              'position',
            ) as KungfuApi.Position[];

          currentPositionList.value = toRaw(
            positions.reverse().map((item) => dealPosition(watcher, item)),
          );
        },
      );

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    currentPositionList,
  };
};

export const useMakeOrderInfo = (
  app: ComponentInternalInstance | null,
  formState: Ref<Record<string, KungfuApi.KfConfigValue>>,
) => {
  const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(
    window.watcher,
  );
  const { currentPositionList } = useCurrentPositionList(app);

  const { getAssetsByKfConfig } = useAssets();

  const instrumentResolved = computed(() => {
    const { instrument } = formState.value;
    return instrument
      ? transformSearchInstrumentResultToInstrument(instrument)
      : null;
  });

  const shotable = (instrumentType: InstrumentTypeEnum): boolean => {
    return instrumentType
      ? ShotableInstrumentTypes.includes(instrumentType)
      : false;
  };

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

  const currentPosition = computed(() => {
    if (!currentPositionList.value.length || !instrumentResolved.value)
      return null;

    const { exchangeId, instrumentId, instrumentType } =
      instrumentResolved.value;
    const targetPositionList: KungfuApi.Position[] =
      currentPositionList.value.filter(
        (position) =>
          position.exchange_id === exchangeId &&
          position.instrument_id === instrumentId &&
          position.instrument_type === instrumentType,
      );

    if (targetPositionList && targetPositionList.length) {
      const { side, offset } = formState.value;

      const targetPositionWithLongDirection = targetPositionList.filter(
        (item) => item.direction === DirectionEnum.Long,
      )[0];
      const targetPositionWithShortDirection = targetPositionList.filter(
        (item) => item.direction === DirectionEnum.Short,
      )[0];

      if (side === SideEnum.Buy) {
        if (offset === OffsetEnum.Open) {
          return targetPositionWithLongDirection;
        } else {
          return targetPositionWithShortDirection;
        }
      } else if (side === SideEnum.Sell) {
        if (offset === OffsetEnum.Open) {
          return targetPositionWithShortDirection;
        } else {
          return targetPositionWithLongDirection;
        }
      }
    }

    return null;
  });

  const currentAvailMoney = computed(() => {
    if (!currentGlobalKfLocation.value) return '--';
    if (!formState.value.account_id) {
      return '--';
    }

    const { source, id } = (
      formState.value.account_id || ''
    ).parseSourceAccountId();
    const tdLocation = isCurrentCategoryIsTd.value
      ? currentGlobalKfLocation.value
      : { category: 'td', group: source, name: id, mode: 'live' };

    const avail = getAssetsByKfConfig(tdLocation).avail;

    return dealKfPrice(avail);
  });

  const currentAvailPosVolume = computed(() => {
    if (!instrumentResolved.value) return '--';

    const { instrumentType } = instrumentResolved.value;
    const { offset } = formState.value;

    if (currentPosition.value) {
      const { yesterday_volume, volume } = currentPosition.value;

      if (shotable(instrumentType)) {
        if (offset === OffsetEnum.CloseYest) {
          return dealKfNumber(yesterday_volume) + '';
        } else if (offset === OffsetEnum.CloseToday) {
          return dealKfNumber(volume - yesterday_volume) + '';
        } else {
          return dealKfNumber(volume) + '';
        }
      } else {
        return dealKfNumber(yesterday_volume) + '';
      }
    }

    return '0';
  });

  function getFutureInstrumentTradeAmount(
    currentPrice: number,
    volume: number,
    instrument,
    direction: DirectionEnum,
  ): number | null {
    if (!instrument) return null;

    const { exchangeId, instrumentId } = instrument;
    const instrumentKey = hashInstrumentUKey(instrumentId, exchangeId);
    const { contract_multiplier, long_margin_ratio, short_margin_ratio } =
      window.watcher.ledger.Instrument[instrumentKey] as KungfuApi.Instrument;

    if (direction === DirectionEnum.Long) {
      return currentPrice * volume * contract_multiplier * long_margin_ratio;
    } else if (direction === DirectionEnum.Short) {
      return currentPrice * volume * contract_multiplier * short_margin_ratio;
    }

    return null;
  }

  function dealTradeAmount(preNumber: number | null) {
    return !Number(preNumber) ? '--' : dealKfPrice(preNumber);
  }

  function getTradeAmount(
    currentPrice: number,
    volume: number,
    currentInstrument?: KungfuApi.InstrumentResolved,
    currentPosition?: KungfuApi.Position,
  ): string | null {
    const instrumentType = currentInstrument?.instrumentType;

    if (instrumentType) {
      if (instrumentType === InstrumentTypeEnum.future) {
        const instrumentTradeAmount = getFutureInstrumentTradeAmount(
          currentPrice,
          volume,
          currentInstrument,
          currentPosition?.direction,
        );
        return dealTradeAmount(instrumentTradeAmount);
      } else if (instrumentType === InstrumentTypeEnum.stock) {
        return dealTradeAmount(currentPrice * volume);
      }
    } else {
      return dealTradeAmount(currentPrice * volume);
    }

    return null;
  }

  const currentPrice = computed(() => {
    const { price_type, limit_price } = formState.value;

    if (price_type === PriceTypeEnum.Limit) {
      return limit_price;
    } else if (price_type === PriceTypeEnum.Market) {
      return currentPosition.value?.last_price;
    }

    return null;
  });

  const currentTradeAmount = computed(() => {
    const { volume, side, account_id } = formState.value;

    if (side === SideEnum.Buy) {
      return getTradeAmount(currentPrice.value || 0, volume);
    } else if (side === SideEnum.Sell) {
      if (
        instrumentResolved.value &&
        (currentGlobalKfLocation.value?.category === 'td' || account_id)
      ) {
        return getTradeAmount(
          currentPrice.value || 0,
          volume,
          instrumentResolved.value.instrumentType,
        );
      }
    }

    return '--';
  });

  const currentResidueMoney = computed(() => {
    const { side } = formState.value;
    if (currentAvailMoney.value !== '--') {
      if (currentTradeAmount.value !== '--') {
        if (side === SideEnum.Buy) {
          return dealKfPrice(
            Number(currentAvailMoney.value) - Number(currentTradeAmount.value),
          );
        } else if (side === SideEnum.Sell) {
          return dealKfPrice(
            Number(currentAvailMoney.value) + Number(currentTradeAmount.value),
          );
        }

        return '--';
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
          if (data.tag === 'open:globalSetting') {
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
