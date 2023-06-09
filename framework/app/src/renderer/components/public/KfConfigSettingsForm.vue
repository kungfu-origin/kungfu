<script setup lang="ts">
import path from 'path';
import { dialog } from '@electron/remote';
import {
  DeleteOutlined,
  DashOutlined,
  CloseOutlined,
  PlusOutlined,
} from '@ant-design/icons-vue';
import {
  getCurrentInstance,
  reactive,
  Ref,
  ref,
  toRaw,
  toRefs,
  watch,
  computed,
  nextTick,
  defineComponent,
} from 'vue';
import {
  BasketVolumeType,
  PriceLevel,
  Side,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  getIdByKfLocation,
  transformSearchInstrumentResultToInstrument,
  numberEnumRadioType,
  numberEnumSelectType,
  stringEnumSelectType,
  KfConfigValueNumberType,
  KfConfigValueArrayType,
  KfConfigValueBooleanType,
  getCombineValueByPrimaryKeys,
  getPriceTypeConfig,
  initFormStateByConfig,
  getPrimaryKeys,
  dealPriceType,
  dealPriceLevel,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { RuleObject } from 'ant-design-vue/lib/form';
import {
  useActiveInstruments,
  useAllKfConfigData,
  useBasket,
  useInstruments,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import dayjs, { Dayjs } from 'dayjs';
import VueI18n, { useLanguage } from '@kungfu-trader/kungfu-js-api/language';
import {
  InstrumentTypeEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  readCSV,
  writeCsvWithUTF8Bom,
} from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  buildInstrumentSelectOptionValue,
  useWritableTableSearchKeyword,
  messagePrompt,
  dealKungfuColorToClassname,
  dealKungfuColorToStyleColor,
} from '../../assets/methods/uiUtils';

const { t } = VueI18n.global;

const props = withDefaults(
  defineProps<{
    formState: Record<string, KungfuApi.KfConfigValue>;
    configSettings: KungfuApi.KfConfigItem[];
    tdList?: KungfuApi.KfLocation[] | null;
    changeType?: KungfuApi.ModalChangeType;
    primaryKeyAvoidRepeatCompareExtra?: string;
    primaryKeyAvoidRepeatCompareTarget?: string[];
    layout?: 'horizontal' | 'vertical' | 'inline';
    labelAlign?: 'right' | 'left';
    labelWrap?: boolean;
    labelCol?: number;
    wrapperCol?: number;
    rules?: Record<
      string,
      {
        type?: string;
        validator: (_rule: RuleObject, value: number | string) => Promise<void>;
        trigger: string;
      }
    >;
    steps?: Record<string, number>;
    passPrimaryKeySpecialWordsVerify?: boolean;
    isPrimaryDisabled?: boolean;
    willReplaceWholeFormState?: boolean;
  }>(),
  {
    formState: () => ({}),
    configSettings: () => [],
    tdList: () => null,
    changeType: 'add',
    primaryKeyAvoidRepeatCompareTarget: () => [],
    primaryKeyAvoidRepeatCompareExtra: '',
    layout: 'horizontal',
    labelAlign: 'right',
    labelWrap: false,
    labelCol: 6,
    wrapperCol: 14,
    rules: () => ({}),
    steps: () => ({}),
    passPrimaryKeySpecialWordsVerify: false,
    isPrimaryDisabled: false,
    willReplaceWholeFormState: false,
  },
);

defineEmits<{
  (
    e: 'update:formState',
    formState: Record<string, KungfuApi.KfConfigValue>,
  ): void;
}>();

type InstrumentsSearchRelated = Record<
  string,
  {
    searchInstrumnetOptions: Ref<{ label: string; value: string }[]>;
    handleSearchInstrument: (value: string) => void;
    updateSearchInstrumnetOptions: (
      type: 'instrument' | 'instruments' | 'instrumentsCsv',
      value: string | string[],
    ) => Promise<{ value: string; label: string }[]>;
    handleSearchInstrumentBlur: () => void;
  }
>;

type TablesSearchRelated = Record<
  string,
  {
    searchKeyword: Ref<string>;
    tableData: Ref<
      {
        data: Record<string, KungfuApi.KfConfigValue>;
        index: number;
        id: string;
      }[]
    >;
  }
>;

const app = getCurrentInstance();
const formRef = ref();

const formState = ref(props.formState);
const { td, md, strategy } = toRefs(useAllKfConfigData());
const { basketList, buildBasketOptionValue } = useBasket();
const { isLanguageKeyAvailable } = useLanguage();

const spinning = ref(false);
const primaryKeys = ref<string[]>(getPrimaryKeys(props.configSettings || []));
const sideRadiosList = ref<string[]>(Object.keys(Side).slice(0, 2));
const customerFormItemTips = reactive<Record<string, string>>({});
const instrumentKeys = ref<
  Record<string, 'instrument' | 'instruments' | 'instrumentsCsv'>
>(filterInstrumentKeysFromConfigSettings(props.configSettings));
const instrumentsCsvData = reactive<
  Record<string, Record<string, KungfuApi.InstrumentResolved>>
>({});
const tableKeys = ref<Record<string, KungfuApi.KfConfigItem>>(
  filterTableKeysFromConfigSettings(props.configSettings),
);

watch(
  () => props.configSettings,
  (newVal) => {
    primaryKeys.value = getPrimaryKeys(newVal);
    instrumentKeys.value = filterInstrumentKeysFromConfigSettings(newVal);
    tableKeys.value = filterTableKeysFromConfigSettings(newVal);

    const rowFormState = toRaw(props.formState);
    Object.keys(rowFormState).forEach(
      (key) => (formState.value[key] = rowFormState[key]),
    );
  },
);

let instrumentsSearchRelated = getInstrumentsSearchRelated(
  instrumentKeys.value,
);
watch(
  () => instrumentKeys.value,
  (newVal) => {
    instrumentsSearchRelated = getInstrumentsSearchRelated(newVal);
  },
);

let tablesSearchRelated = getTablesSearchRelated(tableKeys.value);
watch(
  () => tableKeys.value,
  (newVal) => {
    tablesSearchRelated = getTablesSearchRelated(newVal);
  },
);

const instrumentOptionsReactiveData = reactive<{
  data: Record<string, { value: string; label: string }[]>;
}>({ data: {} });
const instrumentsInFrom = computed(() =>
  Object.keys(instrumentKeys.value).map((key) => ({
    key,
    value: formState.value[key],
  })),
);
watch(instrumentsInFrom, (insts) => {
  // have to be after watch(() => instrumentKeys.value, xxx)
  nextTick().then(() => {
    insts.forEach((item) => {
      instrumentsSearchRelated[item.key]
        .updateSearchInstrumnetOptions(
          instrumentKeys.value[item.key],
          item.value,
        )
        .then((options) => {
          instrumentOptionsReactiveData.data[item.key] = options;
        });
    });
  });
});

if (props.willReplaceWholeFormState) {
  watch(
    () => props.formState,
    (newVal) => {
      formState.value = newVal;
    },
  );
}

watch(
  () => formState.value,
  (newVal) => {
    app && app.emit('update:formState', newVal);
  },
  {
    deep: true,
  },
);

if ('instrument' in formState.value && 'side' in formState.value) {
  watch(
    () => formState.value.instrument,
    (newInstrument: string) => {
      if (newInstrument) {
        const instrumentResolved =
          transformSearchInstrumentResultToInstrument(newInstrument);
        if (instrumentResolved) {
          const { instrumentType } = instrumentResolved;
          if (instrumentType === InstrumentTypeEnum.stockoption) {
            sideRadiosList.value = [
              ...Object.keys(Side).slice(0, 2),
              SideEnum.Exec + '',
            ];
          } else {
            sideRadiosList.value = Object.keys(Side).slice(0, 2);
          }
        }
      }
    },
  );
}

function getInstrumentsSearchRelated(
  instrumentKeys: Record<
    string,
    'instrument' | 'instruments' | 'instrumentsCsv'
  >,
): InstrumentsSearchRelated {
  return Object.keys(instrumentKeys).reduce(
    (item1: InstrumentsSearchRelated, key: string) => {
      const {
        searchInstrumnetOptions,
        handleSearchInstrument,
        updateSearchInstrumnetOptions,
      } = useInstruments();

      updateSearchInstrumnetOptions(
        instrumentKeys[key],
        formState.value[key],
      ).then((options) => {
        instrumentOptionsReactiveData.data[key] = options;
      });

      item1[key] = {
        searchInstrumnetOptions,
        handleSearchInstrument: (val) => {
          handleSearchInstrument(val).then((options) => {
            instrumentOptionsReactiveData.data[key] = options;
          });
        },
        updateSearchInstrumnetOptions,
        handleSearchInstrumentBlur: () => {
          updateSearchInstrumnetOptions(
            instrumentKeys[key],
            formState.value[key],
          ).then((options) => {
            instrumentOptionsReactiveData.data[key] = options;
          });
        },
      };
      return item1;
    },
    {} as InstrumentsSearchRelated,
  );
}

function getTablesSearchRelated(
  tableKeys: Record<string, KungfuApi.KfConfigItem>,
): TablesSearchRelated {
  return Object.keys(tableKeys).reduce((tablesSearchRelated, key) => {
    const targetList = ref(formState.value[key]);
    const keys =
      tableKeys[key].search?.keys ||
      tableKeys[key].headers?.map((header) => header.title) ||
      [];
    const { searchKeyword, tableData } = useWritableTableSearchKeyword<
      Record<string, KungfuApi.KfConfigItem>
    >(targetList, keys);
    tablesSearchRelated[key] = {
      searchKeyword,
      tableData,
    };
    return tablesSearchRelated;
  }, {} as TablesSearchRelated);
}

function getValidatorType(
  type: string,
): 'number' | 'string' | 'array' | 'boolean' {
  if (KfConfigValueNumberType.includes(type)) {
    return 'number';
  } else if (KfConfigValueArrayType.includes(type)) {
    return 'array';
  } else if (KfConfigValueBooleanType.includes(type)) {
    return 'boolean';
  } else {
    return 'string';
  }
}

function filterInstrumentKeysFromConfigSettings(
  configSettings: KungfuApi.KfConfigItem[],
) {
  return configSettings
    .filter(
      (item) =>
        item.type === 'instrument' ||
        item.type === 'instruments' ||
        item.type === 'instrumentsCsv',
    )
    .reduce((data, setting) => {
      data[setting.key.toString()] = setting.type as
        | 'instrument'
        | 'instruments'
        | 'instrumentsCsv';
      return data;
    }, {} as Record<string, 'instrument' | 'instruments' | 'instrumentsCsv'>);
}

function filterTableKeysFromConfigSettings(
  configSettings: KungfuApi.KfConfigItem[],
) {
  return configSettings
    .filter((item) => item.type === 'table' || item.type === 'csvTable')
    .reduce((data, setting) => {
      data[setting.key.toString()] = setting;
      return data;
    }, {} as Record<string, KungfuApi.KfConfigItem>);
}

function isNumberInputType(type: string): boolean {
  const numberInputTypes: string[] = ['int', 'float', 'percent'];
  return numberInputTypes.includes(type);
}

const SpecialWordsReg = new RegExp(
  "[`~!@#$^&*()=|{}';'\\[\\]<>《》?~！@#￥……&*（）——|{}【】‘；”“'。，、？_]",
);
function primaryKeyValidator(_rule: RuleObject, value: string): Promise<void> {
  const combineValue: string = getCombineValueByPrimaryKeys(
    primaryKeys.value,
    formState.value,
    props.primaryKeyAvoidRepeatCompareExtra,
  );
  if (
    props.primaryKeyAvoidRepeatCompareTarget
      .map((item): string => item.toLowerCase())
      .includes(combineValue.toLowerCase())
  ) {
    return Promise.reject(
      new Error(
        t('validate.value_existing', {
          value: combineValue,
        }),
      ),
    );
  }

  if (
    SpecialWordsReg.test(value || '') &&
    !props.passPrimaryKeySpecialWordsVerify
  ) {
    return Promise.reject(new Error(t('validate.no_special_characters')));
  }

  if (
    (value || '').toString().includes('_') &&
    !props.passPrimaryKeySpecialWordsVerify
  ) {
    return Promise.reject(new Error(t('validate.no_underline')));
  }

  return Promise.resolve();
}

function noZeroValidator(_rule: RuleObject, value: number): Promise<void> {
  if (Number.isNaN(+value)) {
    return Promise.reject(new Error(t('validate.no_zero_number')));
  }

  if (+value === 0) {
    return Promise.reject(new Error(t('validate.no_zero_number')));
  }

  // if (+value < 0) {
  //   return Promise.reject(new Error(t('validate.no_negative_number')));
  // }

  return Promise.resolve();
}

function instrumnetValidator(_rule: RuleObject, value: string): Promise<void> {
  if (!value) {
    return Promise.resolve();
  }

  const instrumentResolved = transformSearchInstrumentResultToInstrument(value);
  if (!instrumentResolved) {
    return Promise.reject(new Error(t('instrument_error')));
  }

  return Promise.resolve();
}

function instrumnetsValidator(
  _rule: RuleObject,
  value: string[],
): Promise<void> {
  if (!value) {
    return Promise.resolve();
  }

  const instrumentResolved = value.filter(
    (instrument) => !transformSearchInstrumentResultToInstrument(instrument),
  );
  if (instrumentResolved.length) {
    return Promise.reject(new Error(t('instrument_error')));
  }

  return Promise.resolve();
}

function getKfTradeValueName(
  data: Record<number | string | symbol, KungfuApi.KfTradeValueCommonData>,
  key: number | string,
): string {
  return data[key].name;
}

function instrumentsCsvCallback(
  instruments: KungfuApi.Instrument[],
  errRows: {
    row: number;
    data: (string | number | boolean)[];
  }[],
  targetKey: string,
) {
  const { getInstrumentByIds } = useActiveInstruments();

  if (!instrumentsCsvData[targetKey]) instrumentsCsvData[targetKey] = {};

  instruments.forEach((item) => {
    if (item.exchange_id && item.instrument_id) {
      const ukey = hashInstrumentUKey(item.instrument_id, item.exchange_id);
      const existedInstrument = instrumentsCsvData[targetKey][ukey];
      if (!existedInstrument || !existedInstrument.instrumentName) {
        const instrumentResolved = getInstrumentByIds(
          item.instrument_id,
          item.exchange_id,
          true,
        ) as KungfuApi.InstrumentResolved;

        instrumentsCsvData[targetKey][ukey] = instrumentResolved;
      }
    }
  });

  const resolvedInstruments = Object.values(instrumentsCsvData[targetKey]);

  const sourceLength = instruments.length;
  const resolvedLength = resolvedInstruments.length;
  const failedLength =
    Number(
      /\d/.exec(customerFormItemTips[targetKey]?.split(',')?.[1] || '0')?.[0],
    ) +
    (sourceLength - resolvedLength + errRows.length);
  customerFormItemTips[targetKey] = t('validate.resolved_tip', {
    success: `${resolvedLength}`,
    fail: `${failedLength}`,
    value: t('tradingConfig.instrument'),
  });
  formState.value[targetKey] = resolvedInstruments.map((item) =>
    buildInstrumentSelectOptionValue(item),
  );
  return Promise.resolve();
}

function handleClearInstrumentsCsv(targetKey: string) {
  formState.value[targetKey] = [];
  customerFormItemTips[targetKey] = '';
}

function csvTableCallback(
  columns: KungfuApi.KfConfigItem[],
  mode?: 'reset' | 'add',
) {
  return function (
    data: Record<string, KungfuApi.KfConfigValue>[],
    errRows: {
      row: number;
      data: (string | number | boolean)[];
    }[],
    targetKey: string,
  ) {
    return new Promise<void>((resolve) => {
      if (errRows.length) {
        console.warn('Csv resolve error rows:', errRows);
        messagePrompt().error(
          `${t('settingsFormConfig.import_failed')}: ${t(
            'settingsFormConfig.csv_format_error',
          )}`,
        );
        return resolve();
      }

      if (data.length) {
        if (mode === 'reset') {
          formState.value[targetKey].length = 0;
        }

        const headers = Object.keys(data[0]);
        const isInstrumentHeader =
          headers.includes('instrument_id') && headers.includes('exchange_id');
        const instrumentColumnConfig = columns.find(
          (item) => item.type === 'instrument',
        );
        const shouldResolveInstrument =
          isInstrumentHeader && instrumentColumnConfig;
        nextTick(() => {
          if (shouldResolveInstrument) {
            const { getInstrumentByIds } = useActiveInstruments();

            data.forEach((item) => {
              const instrument = getInstrumentByIds(
                item.instrument_id,
                `${item.exchange_id}`.toUpperCase(),
                true,
              ) as KungfuApi.InstrumentResolved;

              formState.value[targetKey].push({
                ...item,
                [instrumentColumnConfig.key]:
                  buildInstrumentSelectOptionValue(instrument),
              });
            });
          } else {
            formState.value[targetKey].push(...data);
          }
          messagePrompt().success();
          resolve();
        });
      }

      resolve();
    });
  };
}

function buildCsvHeadersDescription(headers: KungfuApi.KfConfigItemHeader[]) {
  return (
    headers
      .map((header) =>
        [
          header.title,
          ...(header.description ? [header.description] : []),
        ].join(': '),
      )
      .join('. ') + '.'
  );
}

function buildCsvHeadersValidator(
  headers: KungfuApi.KfConfigItemHeader[] | undefined,
) {
  if (!headers || !headers.length) return undefined;

  const requiredHeaders: string[] = [];
  headers.forEach((header) => {
    if (header.required) {
      requiredHeaders.push(header.title);
    }
  });

  return (row) => {
    for (let header of headers) {
      if (requiredHeaders.indexOf(header.title) !== -1) {
        if (
          !(
            header.title in row &&
            row[header.title] !== null &&
            row[header.title] !== undefined &&
            row[header.title] !== ''
          )
        ) {
          return false;
        }
      }

      const type = header.type ?? 'str';
      let value = `${row[header.title]}`;

      switch (type) {
        case 'str':
          if (!value) return false;
          break;
        case 'num':
          if (Number.isNaN(Number(value))) return false;
          break;
        case 'precent':
          if (
            !value.endsWith('%') ||
            Number.isNaN(Number(value.replace('%', '')))
          )
            return false;
          break;
        case 'bool':
          value = value.toLowerCase();
          if (value !== 'true' && value !== 'false') return false;
      }
    }

    return true;
  };
}

function buildCsvHeadersTransformer(
  headers: KungfuApi.KfConfigItemHeader[] | undefined,
) {
  if (!headers || !headers.length) return undefined;

  const headerWithDefault: Record<string, KungfuApi.KfConfigValue> = {};
  headers.forEach((header) => {
    if (header.default !== undefined && header.default !== null) {
      headerWithDefault[header.title] = header.default;
    }
  });

  return (row) => {
    for (let header of headers) {
      if (header.title in headerWithDefault) {
        if (
          row[header.title] === null ||
          row[header.title] === undefined ||
          row[header.title] === ''
        ) {
          row[header.title] = headerWithDefault[header.title];
        }
      }

      const type = header.type ?? 'str';
      let value = `${row[header.title]}`;

      switch (type) {
        case 'str':
          row[header.title] = value;
          break;
        case 'num':
          row[header.title] = Number(value);
          break;
        case 'precent':
          row[header.title] = Number(value.replace('%', ''));
          break;
        case 'bool':
          value = value.toLowerCase();
          row[header.title] =
            value === 'false' ? false : value === 'true' ? true : !!value;
          break;
      }
    }

    return row;
  };
}

function handleSelectCsv<T>(
  targetKey: string,
  headers: KungfuApi.KfConfigItemHeader[],
  callback?: (
    data: T[],
    errRows: {
      row: number;
      data: (string | number | boolean)[];
    }[],
    targetKey: string,
  ) => Promise<void>,
): void {
  dialog
    .showOpenDialog({
      properties: ['openFile'],
      filters: [{ name: 'CSV', extensions: ['csv'] }],
    })
    .then((res) => {
      const { filePaths } = res;
      if (filePaths.length) {
        spinning.value = true;
        readCSV<T>(filePaths[0], true, {
          validator: buildCsvHeadersValidator(headers),
          transformer: buildCsvHeadersTransformer(headers),
        })
          .then(({ resRows, errRows }) => {
            if (callback) return callback(resRows, errRows, targetKey);

            return Promise.resolve();
          })
          .catch((err) => {
            messagePrompt().error(
              `${t('settingsFormConfig.import_failed')}: ${t(
                'settingsFormConfig.csv_format_error',
              )}`,
            );
            if (err instanceof Error) {
              console.error(err);
            }
          })
          .finally(() => {
            spinning.value = false;
          });
      }
    });
}

function handleDownloadCsvTemplate(
  templates: KungfuApi.KfConfigItemTemplate[],
) {
  if (templates && Array.isArray(templates)) {
    dialog
      .showOpenDialog({
        title: t('settingsFormConfig.csv_template'),
        properties: ['createDirectory', 'openDirectory'],
      })
      .then(({ filePaths }) => {
        if (filePaths && filePaths.length) {
          Promise.all(
            templates.map((template) => {
              const filePath = path.join(
                filePaths[0],
                template.name || t('settingsFormConfig.csv_template') + '.csv',
              );
              return writeCsvWithUTF8Bom(filePath, template.data || [], true);
            }),
          ).then(() => {
            messagePrompt().success();
          });
        }
      });
  }
}

function handleSelectFile(targetKey: string): void {
  dialog
    .showOpenDialog({
      properties: ['openFile'],
    })
    .then((res) => {
      const { filePaths } = res;
      if (filePaths.length) {
        formState.value[targetKey] = filePaths[0];
        formRef.value.validateFields([targetKey]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
      }
    });
}

function handleSelectFiles(targetKey: string): void {
  dialog
    .showOpenDialog({
      properties: ['openDirectory'],
    })
    .then((res) => {
      const { filePaths } = res;
      if (filePaths.length) {
        (formState.value[targetKey] as string[]).push(filePaths[0]);
        formRef.value.validateFields([targetKey]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
      }
    });
}

function handleRemoveFile(key: string, filename: string): void {
  const index = (formState.value[key] as string[]).indexOf(filename);
  if (index !== -1) {
    (formState.value[key] as string[]).splice(index, 1);
  }
}

function onOpenRangePickerChange(open: boolean, key: string) {
  if (open) {
    formState.value[key] = null;
  }
}

function onRangePickerCalendarChange(val: Dayjs[], key: string) {
  if (val) {
    formState.value[key] = val.map((d) => {
      if (d) {
        return d.format('YYYY-MM-DD HH:mm:ss');
      } else if (val[0]) {
        return val[0].format('YYYY-MM-DD HH:mm:ss');
      } else if (val[1]) {
        return val[1].format('YYYY-MM-DD HH:mm:ss');
      } else {
        return dayjs().format('YYYY-MM-DD HH:mm:ss');
      }
    });
  }
}

function disabledEndDate(currentDate: Dayjs, key: string, timeInterval = 1) {
  if (!formState.value[key] || !formState.value[key][0]) {
    return false;
  }
  let endTime;
  const startTime = dayjs(formState.value[key][0]);
  const hour = startTime.hour();
  const minute = startTime.minute();
  if (hour === 0 && minute === 0) {
    endTime = startTime.add(timeInterval, 'day');
  } else {
    endTime = startTime.add(timeInterval + 1, 'day');
  }

  return (
    currentDate &&
    (currentDate.valueOf() <= startTime.valueOf() ||
      currentDate.valueOf() >= endTime.valueOf())
  );
}

function disabledEndTime(
  currentDate: Dayjs,
  type: string,
  key: string,
  timeInterval = 1,
) {
  if (!formState.value[key] || !formState.value[key][0]) {
    return {};
  }
  if (type === 'start') {
    return {};
  }

  const startTime = dayjs(formState.value[key][0]);
  const endTime = startTime.add(timeInterval, 'day');

  const disabledHours = () => {
    const hours: number[] = [];

    if (currentDate.isSame(startTime, 'day')) {
      for (let i = 0; i <= startTime.hour(); i++) {
        hours.push(i);
      }
    }

    if (currentDate.isSame(endTime, 'day')) {
      for (let i = endTime.hour() + 1; i < 24; i++) {
        hours.push(i);
      }
    }

    return hours;
  };

  const disabledMinutes = (selectedHour) => {
    const minutes: number[] = [];

    if (
      currentDate.isSame(startTime, 'day') &&
      selectedHour === startTime.hour()
    ) {
      for (let i = 0; i <= startTime.minute(); i++) {
        minutes.push(i);
      }
    }

    if (currentDate.isSame(endTime, 'day') && selectedHour === endTime.hour()) {
      for (let i = endTime.minute() + 1; i < 60; i++) {
        minutes.push(i);
      }
    }

    return minutes;
  };

  return {
    disabledHours,
    disabledMinutes,
  };
}

function handleRangePickerChange(date: Dayjs[], key: string) {
  if (date) {
    formState.value[key] = date.map((d) =>
      dayjs(d).toString() === 'Invalid Date'
        ? null
        : dayjs(d).format('YYYY-MM-DD HH:mm:ss'),
    );
  } else {
    formState.value[key] = null;
  }
}

function handleDateTimePickerChange(date: Dayjs, key: string) {
  formState.value[key] =
    dayjs(date).toString() === 'Invalid Date'
      ? null
      : dayjs(date).format('YYYY-MM-DD HH:mm:ss');
}

function handleDatePickerChange(date: Dayjs, key: string) {
  formState.value[key] =
    dayjs(date).toString() === 'Invalid Date'
      ? null
      : dayjs(date).format('YYYY-MM-DD');
}

function handleTimePickerChange(date: Dayjs, key: string) {
  formState.value[key] =
    dayjs(date).toString() === 'Invalid Date'
      ? null
      : dayjs(date).format('YYYY-MM-DD HH:mm:ss');
}

function handleInstrumentSelected(val: string, key: string) {
  if (!formState.value[key].includes(val)) {
    formState.value[key].push(val);
    formRef.value.validateFields([key]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
  }
}

function handleInstrumentDeselected(val: string, key: string) {
  const index = formState.value[key].indexOf(val);
  if (index !== -1) {
    formState.value[key].splice(index, 1);
    formRef.value.validateFields([key]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
  }
}

function validate(): Promise<void> {
  return formRef.value.validate();
}

function clearValidate(): void {
  return formRef.value.clearValidate();
}

function formatterPercentNumber(value: number): string {
  return `${value}%`;
}

function parserPercentString(value: string): number {
  return +Number(value.replace('%', ''));
}

function handleAddItemIntoTableRows(item: KungfuApi.KfConfigItem) {
  const targetState = formState.value[item.key];
  const tmp = initFormStateByConfig(item.columns || [], {});
  if (targetState instanceof Array) {
    targetState.unshift(tmp);
  }
}

function handleRemoveItemIntoTableRows(item, index) {
  const targetState = formState.value[item.key];
  if (targetState instanceof Array) {
    targetState.splice(index, 1);
  }
}

function calcTableItemHeight(
  layout: 'horizontal' | 'vertical' | 'inline',
  noDivider: boolean,
) {
  const baseHeight = layout === 'vertical' ? 52 : 32;
  const dividerHeight = noDivider ? 8 : 25;
  return baseHeight + dividerHeight;
}

defineExpose({
  validate,
  clearValidate,
});
</script>
<template>
  <a-form
    ref="formRef"
    class="kf-config-form"
    :model="formState"
    :label-col="layout === 'inline' ? null : { span: labelCol }"
    :label-wrap="labelWrap"
    :wrapper-col="layout === 'inline' ? null : { span: wrapperCol }"
    :label-align="labelAlign"
    :colon="false"
    :scroll-to-first-error="true"
    :layout="layout"
  >
    <a-form-item
      v-for="item in configSettings"
      :key="item.key"
      :label="isLanguageKeyAvailable(item.name) ? $t(item.name) : item.name"
      :name="item.key"
      :extra="
        item.tip && isLanguageKeyAvailable(item.tip) ? $t(item.tip) : item.tip
      "
      :rules="
        (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
        item.disabled
          ? []
          : [
              ...(rules[item.key]
                ? [
                    {
                      type: getValidatorType(item.type),
                      ...rules[item.key],
                    },
                  ]
                : []),
              ...(item.required
                ? [
                    {
                      required: item.required,
                      type: getValidatorType(item.type),
                      message: item.errMsg
                        ? isLanguageKeyAvailable(item.errMsg)
                          ? $t(item.errMsg)
                          : item.errMsg
                        : $t('validate.mandatory'),
                      trigger: 'blur',
                    },
                  ]
                : [
                    {
                      required: false,
                      type: getValidatorType(item.type),
                      trigger: 'blur',
                    },
                  ]),
              ...(item.required && isNumberInputType(item.type)
                ? [
                    {
                      validator: noZeroValidator,
                      type: getValidatorType(item.type),
                      trigger: 'blur',
                    },
                  ]
                : []),
              ...(item.primary
                ? [
                    {
                      validator: primaryKeyValidator,
                      type: getValidatorType(item.type),
                      trigger: 'change',
                    },
                  ]
                : []),

              ...(item.type === 'instrument'
                ? [
                    {
                      validator: instrumnetValidator,
                      type: getValidatorType(item.type),
                      trigger: 'change',
                    },
                  ]
                : []),

              ...(item.type === 'instruments' || item.type === 'instrumentsCsv'
                ? [
                    {
                      validator: instrumnetsValidator,
                      type: getValidatorType(item.type),
                      trigger: 'change',
                    },
                  ]
                : []),
            ]
      "
    >
      <a-input
        v-if="item.type === 'str'"
        v-model:value.trim="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-input>
      <a-input-password
        v-else-if="item.type === 'password'"
        v-model:value.trim="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-input-password>
      <a-input-number
        v-else-if="item.type === 'int'"
        v-model:value="formState[item.key]"
        :max="item.max ?? Infinity"
        :min="item.min ?? -Infinity"
        :formatter="(val) => Math.floor(val)"
        :parser="(val) => Math.floor(Number(val))"
        :step="steps[item.key] || 1"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-input-number>
      <a-input-number
        v-else-if="item.type === 'float'"
        v-model:value="formState[item.key]"
        :max="item.max ?? Infinity"
        :min="item.min ?? -Infinity"
        :precision="item.precision ?? 4"
        :step="steps[item.key] || 0.0001"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-input-number>
      <a-input-number
        v-else-if="item.type === 'percent'"
        v-model:value="formState[item.key]"
        :max="item.max ?? Infinity"
        :min="item.min ?? -Infinity"
        :precision="item.precision ?? 2"
        :step="steps[item.key] || 0.01"
        :formatter="formatterPercentNumber"
        :parser="parserPercentString"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-input-number>
      <a-radio-group
        v-else-if="item.type === 'side'"
        v-model:value="formState[item.key]"
        :name="item.key"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-radio v-for="key in sideRadiosList" :key="key" :value="+key">
          {{ dealSide(+key).name }}
        </a-radio>
      </a-radio-group>
      <a-select
        v-else-if="item.type === 'priceType'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="key in Object.keys(getPriceTypeConfig()).filter(
            (enumValue) => +enumValue !== PriceTypeEnum.Unknown,
          )"
          :key="key"
          :value="+key"
        >
          {{ dealPriceType(+key).name }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'priceLevel'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="key in Object.keys(PriceLevel).slice(0, 13)"
          :key="key"
          :value="+key"
        >
          {{ dealPriceLevel(+key).name }}
        </a-select-option>
      </a-select>
      <a-radio-group
        v-else-if="numberEnumRadioType[item.type]"
        v-model:value="formState[item.key]"
        :name="item.key"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-radio
          v-for="key in Object.keys(numberEnumRadioType[item.type])"
          :key="key"
          :value="+key"
        >
          {{ getKfTradeValueName(numberEnumRadioType[item.type], key) }}
        </a-radio>
      </a-radio-group>
      <a-radio-group
        v-else-if="item.type === 'radio'"
        v-model:value="formState[item.key]"
        :name="item.key"
        :disabled="
          changeType === 'update' && item.primary && !isPrimaryDisabled
        "
      >
        <a-radio
          v-for="option in item.options"
          :key="option.value"
          :value="option.value"
        >
          <a-tag
            v-if="option.type === 'tag'"
            :color="dealKungfuColorToStyleColor(option.color || 'default')"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </a-tag>
          <span
            v-else
            :class="dealKungfuColorToClassname(option.color || 'text')"
            :style="{
              color: dealKungfuColorToStyleColor(option.color || 'text'),
            }"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </span>
        </a-radio>
      </a-radio-group>
      <a-checkbox
        v-else-if="item.type === 'checkbox'"
        v-model:checked="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-checkbox>
      <a-select
        v-else-if="numberEnumSelectType[item.type]"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        {{ item.type }}
        <a-select-option
          v-for="key in Object.keys(numberEnumSelectType[item.type])"
          :key="key"
          :value="+key"
        >
          {{ getKfTradeValueName(numberEnumSelectType[item.type], key) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="stringEnumSelectType[item.type]"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        {{ item.type }}
        <a-select-option
          v-for="key in Object.keys(stringEnumSelectType[item.type])"
          :key="key"
          :value="key"
        >
          {{ getKfTradeValueName(stringEnumSelectType[item.type], key) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'select'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="option in item.options"
          :key="option.value"
          :value="option.value"
        >
          <a-tag
            v-if="option.type === 'tag'"
            :color="dealKungfuColorToStyleColor(option.color || 'default')"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </a-tag>
          <span
            v-else
            :class="dealKungfuColorToClassname(option.color || 'text')"
            :style="{
              color: dealKungfuColorToStyleColor(option.color || 'text'),
            }"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </span>
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'multiSelect'"
        v-model:value="formState[item.key]"
        mode="multiple"
        :filter-option="
          (inputValue, option) =>
            option.key.toLowerCase().indexOf(inputValue.toLowerCase()) > -1
        "
        allow-clear
        show-search
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="option in item.options"
          :key="option.label"
          :value="option.value"
        >
          <a-tag
            v-if="option.type === 'tag'"
            :color="dealKungfuColorToStyleColor(option.color || 'default')"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </a-tag>
          <span
            v-else
            :class="dealKungfuColorToClassname(option.color || 'text')"
            :style="{
              color: dealKungfuColorToStyleColor(option.color || 'text'),
            }"
          >
            {{
              isLanguageKeyAvailable(option.label + '')
                ? $t(option.label + '')
                : option.label
            }}
          </span>
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'instrument'"
        :ref="item.key"
        v-model:value="formState[item.key]"
        class="instrument-select"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        show-search
        :filter-option="false"
        :options="instrumentOptionsReactiveData.data[item.key]"
        @search="instrumentsSearchRelated[item.key].handleSearchInstrument"
        @blur="instrumentsSearchRelated[item.key].handleSearchInstrumentBlur"
      ></a-select>
      <a-select
        v-else-if="item.type === 'instruments'"
        :ref="item.key"
        class="instrument-select"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        mode="multiple"
        show-search
        :value="formState[item.key]"
        :filter-option="false"
        :options="instrumentOptionsReactiveData.data[item.key]"
        @search="instrumentsSearchRelated[item.key].handleSearchInstrument"
        @select="handleInstrumentSelected($event, item.key)"
        @deselect="handleInstrumentDeselected($event, item.key)"
        @blur="instrumentsSearchRelated[item.key].handleSearchInstrumentBlur"
      ></a-select>

      <a-select
        v-else-if="item.type === 'td'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="config in tdList ? tdList : td"
          :key="getIdByKfLocation(config)"
          :value="getIdByKfLocation(config)"
        >
          {{ getIdByKfLocation(config) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'tds'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        mode="multiple"
        show-search
      >
        <a-select-option
          v-for="config in td"
          :key="getIdByKfLocation(config)"
          :value="getIdByKfLocation(config)"
        >
          {{ getIdByKfLocation(config) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'md'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="config in md"
          :key="getIdByKfLocation(config)"
          :value="getIdByKfLocation(config)"
        >
          {{ getIdByKfLocation(config) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'strategy'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="config in strategy"
          :key="getIdByKfLocation(config)"
          :value="getIdByKfLocation(config)"
        >
          {{ getIdByKfLocation(config) }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'basket'"
        v-model:value="formState[item.key]"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      >
        <a-select-option
          v-for="basket in basketList"
          :key="basket.id"
          :value="buildBasketOptionValue(basket)"
        >
          <span>
            {{ basket.name }}
            <a-tag
              style="margin-left: 4px"
              :color="BasketVolumeType[basket.volume_type].color"
            >
              {{ BasketVolumeType[basket.volume_type].name }}
            </a-tag>
          </span>
        </a-select-option>
      </a-select>
      <a-switch
        v-else-if="item.type === 'bool'"
        v-model:checked="formState[item.key]"
        size="small"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
      ></a-switch>
      <div v-else-if="item.type === 'file'" class="kf-form-item__warp file">
        <a-button
          size="small"
          :disabled="
            (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
            item.disabled
          "
          @click="handleSelectFile(item.key)"
        >
          <template #icon><DashOutlined /></template>
        </a-button>
        <div
          v-if="formState[item.key]"
          class="file-path"
          :title="(formState[item.key] || '').toString()"
        >
          <span class="name">{{ formState[item.key] }}</span>
        </div>
      </div>
      <div
        v-else-if="item.type === 'instrumentsCsv'"
        class="kf-form-item__warp instruments-csv__wrap"
      >
        <a-select
          class="instrument-select"
          :value="formState[item.key]"
          :disabled="
            (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
            item.disabled
          "
          mode="multiple"
          :max-tag-count="5"
          show-search
          allow-clear
          :filter-option="false"
          :options="instrumentOptionsReactiveData.data[item.key]"
          @search="instrumentsSearchRelated[item.key].handleSearchInstrument"
          @select="handleInstrumentSelected($event, item.key)"
          @deselect="handleInstrumentDeselected($event, item.key)"
          @blur="instrumentsSearchRelated[item.key].handleSearchInstrumentBlur"
        ></a-select>
        <div class="select-csv-button__wrap">
          <div class="select-csv-buttons">
            <a-button
              size="small"
              :disabled="
                (changeType === 'update' &&
                  item.primary &&
                  !isPrimaryDisabled) ||
                item.disabled
              "
              @click="
                handleSelectCsv<KungfuApi.Instrument>(
                  item.key,
                  item.headers || [],
                  instrumentsCsvCallback,
                )
              "
            >
              {{ $t('settingsFormConfig.add_csv') }}
            </a-button>
            <a-button
              v-if="!!item.template"
              size="small"
              :disabled="
                (changeType === 'update' &&
                  item.primary &&
                  !isPrimaryDisabled) ||
                item.disabled
              "
              @click="handleDownloadCsvTemplate(item.template || [])"
            >
              {{ $t('settingsFormConfig.csv_template') }}
            </a-button>
          </div>
          <span v-if="item.headers" class="select-csv-tip">
            {{
              $t('settingsFormConfig.add_csv_desc', {
                header: buildCsvHeadersDescription(item.headers),
              })
            }}
          </span>
        </div>
        <div
          v-if="customerFormItemTips[item.key]"
          class="csv-resolved-desc"
          :title="(customerFormItemTips[item.key] || '').toString()"
        >
          <span class="name">{{ customerFormItemTips[item.key] }}</span>
          <span class="clear" @click="handleClearInstrumentsCsv(item.key)">
            {{ $t('settingsFormConfig.clear') }}
          </span>
        </div>
      </div>
      <div v-else-if="item.type === 'files'" class="kf-form-item__warp file">
        <a-button
          :disabled="
            (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
            item.disabled
          "
          size="small"
          @click="handleSelectFiles(item.key)"
        >
          <template #icon><PlusOutlined /></template>
        </a-button>
        <template v-if="formState[item.key]">
          <div
            v-for="file in formState[item.key] as string[] || []"
            :key="file"
            class="file-path"
            :title="file"
          >
            <span class="name">{{ file }}</span>
            <CloseOutlined
              v-if="!(item.default as string[]).includes(file)"
              class="kf-hover"
              @click="handleRemoveFile(item.key, file)"
            />
          </div>
        </template>
      </div>
      <a-range-picker
        v-else-if="item.type === 'rangePicker'"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        :disabled-date="
          (currentDate) =>
            disabledEndDate(currentDate, item.key, item.disableDateRange)
        "
        :disabled-time="
          (currentDate, type) =>
            disabledEndTime(currentDate, type, item.key, item.disableDateRange)
        "
        :show-time="{
          hideDisabledOptions: true,
          defaultValue: [
            dayjs('00:00:00', 'HH:mm:ss'),
            dayjs('11:59:59', 'HH:mm:ss'),
          ],
        }"
        :value="Array.isArray(formState[item.key]) ? formState[item.key].map((item: string) => dayjs(item)) : null"
        @open-change="
          onOpenRangePickerChange($event as unknown as boolean, item.key)
        "
        @calendar-change="
          onRangePickerCalendarChange($event as unknown as Dayjs[], item.key)
        "
        @change="
          handleRangePickerChange($event as unknown as Dayjs[], item.key)
        "
      ></a-range-picker>
      <a-date-picker
        v-else-if="item.type === 'dateTimePicker'"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        format="YYYY-MM-DD HH:mm:ss"
        :show-time="{ defaultValue: dayjs('00:00:00', 'HH:mm:ss') }"
        :value="
          formState[item.key] == null || formState[item.key] == ''
            ? null
            : dayjs(formState[item.key])
        "
        @change="
          handleDateTimePickerChange($event as unknown as Dayjs, item.key)
        "
      ></a-date-picker>
      <a-date-picker
        v-else-if="item.type === 'datePicker'"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        :value="
          formState[item.key] == null || formState[item.key] == ''
            ? null
            : dayjs(formState[item.key])
        "
        @change="handleDatePickerChange($event as unknown as Dayjs, item.key)"
      ></a-date-picker>
      <a-time-picker
        v-else-if="item.type === 'timePicker'"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        :value="formState[item.key] == null ? null : dayjs(formState[item.key])"
        @change="handleTimePickerChange($event as unknown as Dayjs, item.key)"
      ></a-time-picker>
      <div
        v-else-if="item.type === 'table' || item.type === 'csvTable'"
        class="table-in-config-setting-form"
      >
        <div v-if="item.type === 'csvTable'" class="select-csv-button__wrap">
          <div class="select-csv-buttons">
            <a-button
              :disabled="
                (changeType === 'update' &&
                  item.primary &&
                  !isPrimaryDisabled) ||
                item.disabled
              "
              @click="
                handleSelectCsv<Record<string, KungfuApi.KfConfigValue>>(
                  item.key,
                  item.headers || [],
                  csvTableCallback(item.columns || [], item.importMode),
                )
              "
            >
              {{ $t('settingsFormConfig.add_csv') }}
            </a-button>
            <a-button
              v-if="!!item.template"
              :disabled="
                (changeType === 'update' &&
                  item.primary &&
                  !isPrimaryDisabled) ||
                item.disabled
              "
              @click="handleDownloadCsvTemplate(item.template || [])"
            >
              {{ $t('settingsFormConfig.csv_template') }}
            </a-button>
          </div>
          <span v-if="!!item.headers" class="select-csv-tip">
            {{
              $t('settingsFormConfig.add_csv_desc', {
                header: buildCsvHeadersDescription(item.headers),
              })
            }}
          </span>
        </div>

        <div class="table-in-config-setting-form-head">
          <a-input-search
            v-if="!!item.search"
            v-model:value="tablesSearchRelated[item.key].searchKeyword.value"
            class="table-in-config-setting-search"
            :placeholder="$t('settingsFormConfig.keyword')"
          />
          <a-button
            :disabled="
              (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
              item.disabled
            "
          >
            <template #icon>
              <PlusOutlined @click.stop="handleAddItemIntoTableRows(item)" />
            </template>
          </a-button>
          <div
            v-if="item.type === 'csvTable' && !!item.search"
            class="table-in-config-setting-total"
          >
            {{
              $t('settingsFormConfig.total', {
                sum: formState[item.key]?.length ?? 0,
              })
            }}
          </div>
        </div>
        <template v-if="!!item.search">
          <RecycleScroller
            v-if="
              tablesSearchRelated[item.key].tableData.value &&
              tablesSearchRelated[item.key].tableData.value.length
            "
            :style="{
              maxHeight: `${
                calcTableItemHeight(layout, !!item.noDivider) * 10
              }px`,
              overflowY: 'overlay',
            }"
            :items="tablesSearchRelated[item.key].tableData.value"
            :item-size="calcTableItemHeight(layout, !!item.noDivider)"
            key-field="id"
            :buffer="0"
          >
            <template
              #default="{
                item: _item,
                index,
              }: {
                item: {
                  data: Record<string, KungfuApi.KfConfigValue>,
                  index: number,
                  id: string,
                },
                index: number,
              }"
            >
              <div
                class="table-in-config-setting-row"
                :style="{
                  paddingBottom: item.noDivider ? '8px' : '',
                }"
              >
                <div class="table-in-config-setting-row-from__wrap">
                  <KfConfigSettingsForm
                    v-model:formState="_item.data"
                    :style="{
                      flexWrap: item.wrap || '',
                      overflowX: item.wrap === 'nowrap' ? 'overlay' : '',
                    }"
                    :config-settings="item.columns || []"
                    :change-type="changeType"
                    :primary-key-avoid-repeat-compare-extra="
                      primaryKeyAvoidRepeatCompareExtra
                    "
                    :primary-key-avoid-repeat-compare-target="
                      primaryKeyAvoidRepeatCompareTarget
                    "
                    layout="inline"
                    :label-align="labelAlign"
                    :label-wrap="labelWrap"
                    :label-col="labelCol"
                    :wrapper-col="wrapperCol"
                    :rules="rules"
                    :steps="steps"
                    :pass-primary-key-special-words-verify="
                      passPrimaryKeySpecialWordsVerify
                    "
                    :is-primary-disabled="isPrimaryDisabled"
                    :will-replace-whole-form-state="true"
                  ></KfConfigSettingsForm>
                  <div class="table-in-config-setting-row-buttons__wrap">
                    <a-button
                      size="small"
                      :disabled="
                        (changeType === 'update' &&
                          item.primary &&
                          !isPrimaryDisabled) ||
                        item.disabled
                      "
                    >
                      <template #icon>
                        <DeleteOutlined
                          @click="
                            handleRemoveItemIntoTableRows(item, _item.index)
                          "
                        />
                      </template>
                    </a-button>
                  </div>
                </div>
                <div
                  v-if="
                    index !==
                      tablesSearchRelated[item.key].tableData.value.length -
                        1 && !item.noDivider
                  "
                  class="table-in-config-setting-row-divider"
                >
                  <a-divider></a-divider>
                </div>
              </div>
            </template>
          </RecycleScroller>
        </template>
        <template v-else>
          <div
            v-for="(_item, index) in formState[item.key]"
            :key="`${index}_${formState[item.key].length}`"
            class="table-in-config-setting-row"
          >
            <div class="table-in-config-setting-row-from__wrap">
              <KfConfigSettingsForm
                v-model:formState="formState[item.key][index]"
                :config-settings="item.columns || []"
                :change-type="changeType"
                :primary-key-avoid-repeat-compare-extra="
                  primaryKeyAvoidRepeatCompareExtra
                "
                :primary-key-avoid-repeat-compare-target="
                  primaryKeyAvoidRepeatCompareTarget
                "
                layout="inline"
                :label-align="labelAlign"
                :label-wrap="labelWrap"
                :label-col="labelCol"
                :wrapper-col="wrapperCol"
                :rules="rules"
                :steps="steps"
                :pass-primary-key-special-words-verify="
                  passPrimaryKeySpecialWordsVerify
                "
                :is-primary-disabled="isPrimaryDisabled"
              ></KfConfigSettingsForm>
              <div class="table-in-config-setting-row-buttons__wrap">
                <a-button
                  size="small"
                  :disabled="
                    (changeType === 'update' &&
                      item.primary &&
                      !isPrimaryDisabled) ||
                    item.disabled
                  "
                >
                  <template #icon>
                    <DeleteOutlined
                      @click="handleRemoveItemIntoTableRows(item, index)"
                    />
                  </template>
                </a-button>
              </div>
            </div>
            <div
              v-if="
                index !==
                  tablesSearchRelated[item.key].tableData.value.length - 1 &&
                !item.noDivider
              "
              class="table-in-config-setting-row-divider"
            >
              <a-divider></a-divider>
            </div>
          </div>
        </template>
      </div>
    </a-form-item>
    <Teleport to="body">
      <a-spin class="kf-config-setting-form-spin" :spinning="spinning"></a-spin>
    </Teleport>
  </a-form>
</template>
<script lang="ts">
export default defineComponent({
  name: 'KfConfigSettingsForm',
});
</script>
<style lang="less">
.kf-config-form {
  .kf-form-item__warp {
    &.file {
      padding-bottom: 4px;

      div.file-path {
        word-break: break-word;
        margin-top: 8px;
        box-sizing: border-box;

        .name {
          padding-right: 16px;
          box-sizing: border-box;
        }
      }

      button {
        width: 40px;
      }
    }

    &.instruments-csv__wrap {
      .csv-resolved-desc {
        word-break: break-word;
        margin-top: 8px;
        box-sizing: border-box;

        .name {
          padding-right: 16px;
          box-sizing: border-box;
        }

        .clear {
          color: #faad14;
          cursor: pointer;
        }

        .select-csv-button__wrap {
          margin-top: 8px;
        }
      }
    }
  }

  .select-csv-button__wrap {
    .select-csv-buttons {
      display: flex;

      button {
        margin-right: 8px;
        width: fit-content;
      }
    }

    .select-csv-tip {
      display: block;
      margin: 4px 0;
      color: grey;
      word-break: break-word;
      user-select: text;
    }
  }

  .table-in-config-setting-form {
    .table-in-config-setting-form-head {
      display: flex;
      margin-bottom: 12px;

      .table-in-config-setting-search {
        width: 200px;
        margin-right: 16px;

        .ant-input {
          height: 32px;
        }
      }

      .table-in-config-setting-total {
        display: flex;
        align-items: center;
        margin-left: 16px;
      }
    }

    .table-in-config-setting-row {
      padding-right: 12px;

      .table-in-config-setting-row-from__wrap {
        display: flex;
        justify-content: space-between;

        .ant-form::-webkit-scrollbar {
          height: 4px;
        }

        .ant-form {
          box-sizing: border-box;

          &.ant-form-inline {
            .ant-row.ant-form-item {
              margin-bottom: 4px;

              .ant-select {
                min-width: 120px;
              }
            }

            .ant-form-item-label > label,
            .global-setting-item .label {
              font-size: 12px;
            }
          }
        }

        .table-in-config-setting-row-buttons__wrap {
          display: flex;
          padding-left: 24px;
        }
      }

      .table-in-config-setting-row-divider {
        width: 100%;
        min-width: 100%;
        padding: 12px 0;

        .ant-divider-horizontal {
          margin: 0;
        }
      }
    }
  }

  .ant-form-item-control-input-content {
    text-align: left;
  }

  .ant-radio-group {
    .ant-radio-wrapper {
      line-height: 2;
    }
  }

  .ant-form-item-extra {
    white-space: pre-line;
  }

  .ant-form-item-label {
    overflow: inherit;
    white-space: normal;
  }
}

.kf-config-setting-form-spin {
  z-index: 9999;
}
</style>
