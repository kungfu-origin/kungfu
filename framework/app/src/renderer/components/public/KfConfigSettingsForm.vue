<script setup lang="ts">
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
  useInstruments,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import dayjs, { Dayjs } from 'dayjs';
import VueI18n, { useLanguage } from '@kungfu-trader/kungfu-js-api/language';
import {
  InstrumentTypeEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { readCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  buildInstrumentSelectOptionValue,
  useWritableTableSearchKeyword,
} from '../../assets/methods/uiUtils';
import { getPriceTypeConfig } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const { t } = VueI18n.global;

const props = withDefaults(
  defineProps<{
    formState: Record<string, KungfuApi.KfConfigValue>;
    configSettings: KungfuApi.KfConfigItem[];
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
  }>(),
  {
    formState: () => ({}),
    configSettings: () => [],
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
      { data: Record<string, KungfuApi.KfConfigValue>; index: number }[]
    >;
  }
>;

const app = getCurrentInstance();
const formRef = ref();

const formState = reactive(props.formState);
const { td, md, strategy } = toRefs(useAllKfConfigData());
const { isLanguageKeyAvailable } = useLanguage();

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
      (key) => (formState[key] = rowFormState[key]),
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
    value: formState[key],
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

watch(formState, (newVal) => {
  app && app.emit('update:formState', newVal);
});

if ('instrument' in formState && 'side' in formState) {
  watch(
    () => formState.instrument,
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

      updateSearchInstrumnetOptions(instrumentKeys[key], formState[key]).then(
        (options) => {
          instrumentOptionsReactiveData.data[key] = options;
        },
      );

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
            formState[key],
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
    const { searchKeyword, tableData } = useWritableTableSearchKeyword<
      Record<string, KungfuApi.KfConfigItem>
    >(formState[key], tableKeys[key].headers || []);
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
  "[`~!@#$^&*()=|{}';',\\[\\]<>《》/?~！@#￥……&*（）——|{}【】‘；：”“'。，、？]",
);
function primaryKeyValidator(_rule: RuleObject, value: string): Promise<void> {
  const combineValue: string = getCombineValueByPrimaryKeys(
    primaryKeys.value,
    formState,
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
  formState[targetKey] = resolvedInstruments.map((item) =>
    buildInstrumentSelectOptionValue(item),
  );
}

function handleClearInstrumentsCsv(targetKey: string) {
  formState[targetKey] = [];
  customerFormItemTips[targetKey] = '';
}

function csvTableCallback(columns: KungfuApi.KfConfigItem[]) {
  return function (
    data: Record<string, KungfuApi.KfConfigValue>[],
    errRows: {
      row: number;
      data: (string | number | boolean)[];
    }[],
    targetKey: string,
  ) {
    console.error(errRows);
    if (data.length) {
      const headers = Object.keys(data[0]);
      const isInstrumentHeader =
        'instrument_id' in headers && 'exchange_id' in headers;
      const instrumentColumnConfig = columns.find(
        (item) => item.type === 'instrument',
      );
      const shouldResolveInstrument =
        isInstrumentHeader && instrumentColumnConfig;

      if (shouldResolveInstrument) {
        const { getInstrumentByIds } = useActiveInstruments();

        formState[targetKey] = data.map((item) => {
          const instrument = getInstrumentByIds(
            item.instrument_id,
            item.exchange_id,
            true,
          ) as KungfuApi.InstrumentResolved;

          return {
            ...item,
            [instrumentColumnConfig.key]:
              buildInstrumentSelectOptionValue(instrument),
          };
        });
      } else {
        formState[targetKey] = data;
      }
    }
  };
}

function handleSelectCsv<T>(
  targetKey: string,
  headers?: string[],
  callback?: (
    data: T[],
    errRows: {
      row: number;
      data: (string | number | boolean)[];
    }[],
    targetKey: string,
  ) => void,
): void {
  dialog
    .showOpenDialog({
      properties: ['openFile'],
      filters: [{ name: 'CSV', extensions: ['csv'] }],
    })
    .then((res) => {
      const { filePaths } = res;
      if (filePaths.length) {
        readCSV<T>(filePaths[0], headers)
          .then(({ resRows, errRows }) => {
            callback && callback(resRows, errRows, targetKey);
          })
          .catch((err) => {
            if (err instanceof Error) {
              console.error(err);
            }
          });
      }
    });
}

function handleSelectFile(targetKey: string): void {
  dialog
    .showOpenDialog({
      properties: ['openFile'],
    })
    .then((res) => {
      const { filePaths } = res;
      if (filePaths.length) {
        formState[targetKey] = filePaths[0];
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
        (formState[targetKey] as string[]).push(filePaths[0]);
        formRef.value.validateFields([targetKey]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
      }
    });
}

function handleRemoveFile(key: string, filename: string): void {
  const index = (formState[key] as string[]).indexOf(filename);
  if (index !== -1) {
    (formState[key] as string[]).splice(index, 1);
  }
}

function handleTimePickerChange(date: Dayjs, key: string) {
  formState[key] =
    dayjs(date).toString() === 'Invalid Date'
      ? null
      : dayjs(date).format('YYYY-MM-DD HH:mm:ss');
}

function handleInstrumentSelected(val: string, key: string) {
  if (!formState[key].includes(val)) {
    formState[key].push(val);
    formRef.value.validateFields([key]); //手动进行再次验证, 因数据放在span中, 改变数据后无法触发验证
  }
}

function handleInstrumentDeselected(val: string, key: string) {
  const index = formState[key].indexOf(val);
  if (index !== -1) {
    formState[key].splice(index, 1);
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
  const targetState = formState[item.key];
  const tmp = initFormStateByConfig(item.columns || [], {});
  if (targetState instanceof Array) {
    targetState.push(tmp);
  }
}

function handleRemoveItemIntoTableRows(item, index) {
  const targetState = formState[item.key];
  if (targetState instanceof Array) {
    targetState.splice(index, 1);
  }
}

defineExpose({
  validate,
  clearValidate,
});
</script>
<template>
  <a-form
    class="kf-config-form"
    ref="formRef"
    :model="formState"
    :labelCol="layout === 'inline' ? null : { span: labelCol }"
    :label-wrap="labelWrap"
    :wrapperCol="layout === 'inline' ? null : { span: wrapperCol }"
    :labelAlign="labelAlign"
    :colon="false"
    :scrollToFirstError="true"
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
        v-model:value="formState[item.key]"
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
        :precision="4"
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
        :precision="2"
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
          {{
            isLanguageKeyAvailable(option.label + '')
              ? $t(option.label + '')
              : option.label
          }}
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
          {{
            isLanguageKeyAvailable(option.label + '')
              ? $t(option.label + '')
              : option.label
          }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'instrument'"
        :ref="item.key"
        class="instrument-select"
        :disabled="
          (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
          item.disabled
        "
        show-search
        v-model:value="formState[item.key]"
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
      <a-switch
        size="small"
        v-else-if="item.type === 'bool'"
        v-model:checked="formState[item.key]"
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
          <a-button
            size="small"
            :disabled="
              (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
              item.disabled
            "
            @click="
              handleSelectCsv<KungfuApi.Instrument>(
                item.key,
                item.headers,
                instrumentsCsvCallback,
              )
            "
          >
            {{ $t('settingsFormConfig.add_csv') }}
          </a-button>
          <span v-if="item.headers" class="select-csv-tip">
            {{
              $t('settingsFormConfig.add_csv_desc', {
                header: item.headers.join(', '),
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
          <a-button
            size="small"
            :disabled="
              (changeType === 'update' && item.primary && !isPrimaryDisabled) ||
              item.disabled
            "
            @click="
              handleSelectCsv<Record<string, KungfuApi.KfConfigValue>>(
                item.key,
                item.headers,
                csvTableCallback(item.columns || []),
              )
            "
          >
            {{ $t('settingsFormConfig.add_csv') }}
          </a-button>
          <span v-if="item.headers" class="select-csv-tip">
            {{
              $t('settingsFormConfig.add_csv_desc', {
                header: item.headers.join(', '),
              })
            }}
          </span>
        </div>

        <div class="table-in-config-setting-form-head">
          <a-input-search
            v-if="!!item.searchable"
            v-model:value="tablesSearchRelated[item.key].searchKeyword"
            class="table-in-config-setting-search"
            :placeholder="$t('globalSettingConfig.varieties')"
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
        </div>
        <template v-if="!!item?.searchable">
          <div
            v-for="(_item, index) in tablesSearchRelated[item.key].tableData
              .value"
            :key="`${index}_${
              tablesSearchRelated[item.key].tableData.value.length
            }`"
            class="table-in-config-setting-row"
          >
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
                    handleRemoveItemIntoTableRows(
                      item,
                      tablesSearchRelated[item.key].tableData.value[index]
                        .index,
                    )
                  "
                />
              </template>
            </a-button>
            <KfConfigSettingsForm
              v-model:formState="
                tablesSearchRelated[item.key].tableData.value[index].data
              "
              :configSettings="item.columns || []"
              :changeType="changeType"
              :rules="rules"
              layout="inline"
            ></KfConfigSettingsForm>
            <a-divider
              v-if="
                index !==
                tablesSearchRelated[item.key].tableData.value.length - 1
              "
            ></a-divider>
          </div>
        </template>
        <template v-else>
          <div
            class="table-in-config-setting-row"
            v-for="(_item, index) in formState[item.key]"
            :key="`${index}_${formState[item.key].length}`"
          >
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
            <KfConfigSettingsForm
              v-model:formState="formState[item.key][index]"
              :configSettings="item.columns || []"
              :changeType="changeType"
              :rules="rules"
              layout="inline"
            ></KfConfigSettingsForm>
            <a-divider
              v-if="index !== formState[item.key].length - 1"
            ></a-divider>
          </div>
        </template>
      </div>
    </a-form-item>
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
      }
    }
  }

  .select-csv-button__wrap {
    margin-top: 8px;

    button {
      width: fit-content;
    }

    .select-csv-tip {
      display: block;
      margin: 4px 0;
      color: grey;
      word-break: break-all;
      user-select: text;
    }
  }

  .table-in-config-setting-form {
    .table-in-config-setting-head {
      display: flex;

      .table-in-config-setting-search {
        margin-right: 16px;
      }
    }

    .table-in-config-setting-row {
      margin-top: 8px;

      > .ant-btn {
        float: right;
      }

      .ant-form {
        padding-right: 60px;
        box-sizing: border-box;

        &.ant-form-inline {
          .ant-row.ant-form-item {
            margin-bottom: 8px;

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

      .ant-divider-horizontal {
        margin: 12px 0;
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
</style>
