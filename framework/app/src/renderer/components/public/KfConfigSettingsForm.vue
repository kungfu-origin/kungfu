<script setup lang="ts">
import { dialog } from '@electron/remote';
import { DashOutlined, CloseOutlined } from '@ant-design/icons-vue';
import {
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  buildInstrumentSelectOptionLabel,
  buildInstrumentSelectOptionValue,
  useAllKfConfigData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getCurrentInstance,
  nextTick,
  reactive,
  Ref,
  ref,
  toRaw,
  toRefs,
  watch,
} from 'vue';
import {
  PriceType,
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
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { RuleObject } from 'ant-design-vue/lib/form';
import { useInstruments } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import dayjs, { Dayjs } from 'dayjs';

const props = withDefaults(
  defineProps<{
    formState: Record<string, KungfuApi.KfConfigValue>;
    configSettings: KungfuApi.KfConfigItem[];
    changeType?: KungfuApi.ModalChangeType;
    primaryKeyAvoidRepeatCompareExtra?: string;
    primaryKeyAvoidRepeatCompareTarget?: string[];
    layout?: 'horizontal' | 'vertical' | 'inline';
    labelAlign?: 'right' | 'left';
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
    primaryKeyUnderline?: boolean;
  }>(),
  {
    formState: () => ({}),
    configSettings: () => [],
    changeType: 'add',
    primaryKeyAvoidRepeatCompareTarget: () => [],
    primaryKeyAvoidRepeatCompareExtra: '',
    layout: 'horizontal',
    labelAlign: 'right',
    labelCol: 6,
    wrapperCol: 16,
    rules: () => ({}),
    primaryKeyUnderline: false,
  },
);

defineEmits<{
  (
    e: 'update:formState',
    formState: Record<string, KungfuApi.KfConfigValue>,
  ): void;
}>();

const app = getCurrentInstance();
const formRef = ref();
const formState = reactive(props.formState);

const primaryKeys: string[] = (props.configSettings || [])
  .filter((item) => item.primary)
  .map((item) => item.key);

const { td, md, strategy } = toRefs(useAllKfConfigData());

const instrumentKeys = props.configSettings
  .filter((item) => item.type === 'instrument' || item.type === 'instruments')
  .reduce((data, setting) => {
    data[setting.key.toString()] = setting.type as 'instrument' | 'instruments';
    return data;
  }, {} as Record<string, 'instrument' | 'instruments'>);

type InstrumentsSearchRelated = Record<
  string,
  {
    searchInstrumnetOptions: Ref<{ label: string; value: string }[]>;
    handleSearchInstrument: (value: string) => void;
  }
>;

const instrumentsSearchRelated = Object.keys(instrumentKeys).reduce(
  (item1: InstrumentsSearchRelated, key: string) => {
    const { searchInstrumnetOptions, handleSearchInstrument } =
      useInstruments();
    searchInstrumnetOptions.value = makeSearchOptionFormInstruments(
      key,
      instrumentKeys[key],
      formState[key],
    );
    item1[key] = {
      searchInstrumnetOptions: searchInstrumnetOptions,
      handleSearchInstrument,
    };
    return item1;
  },
  {} as InstrumentsSearchRelated,
);

Object.keys(instrumentKeys).forEach((key) => {
  watch(
    () => formState[key],
    (newVal) => {
      nextTick().then(() => {
        instrumentsSearchRelated[key].searchInstrumnetOptions.value =
          makeSearchOptionFormInstruments(key, instrumentKeys[key], newVal);
      });
    },
  );
});

watch(formState, (newVal) => {
  app && app.emit('update:formState', newVal);
});

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

function makeSearchOptionFormInstruments(
  key: string,
  type: 'instrument' | 'instruments',
  value: string | string[],
): { value: string; label: string }[] {
  const valResolved = resolveInstrumentValue(type, value);
  const instrumentResolveds: Array<KungfuApi.InstrumentResolved> = valResolved
    .map((item) => {
      return transformSearchInstrumentResultToInstrument(item.toString());
    })
    .filter((item) => !!item);

  if (!instrumentResolveds.length) {
    if (type === 'instruments') {
      formState[key] = [];
    } else {
      formState[key] = '';
    }
  }

  return [
    ...instrumentResolveds.map((item) => ({
      value: buildInstrumentSelectOptionValue(item),
      label: buildInstrumentSelectOptionLabel(item),
    })),
  ];
}

function resolveInstrumentValue(
  type: 'instrument' | 'instruments',
  value: string | string[],
): string[] {
  if (type === 'instruments') {
    return value as string[];
  }
  if (type === 'instrument') {
    return [value as string];
  } else {
    return [];
  }
}

function isNumberInputType(type: string): boolean {
  const numberInputTypes: string[] = ['int', 'float', 'percent'];
  return numberInputTypes.includes(type);
}

const SpecialWordsReg = new RegExp(
  "[`~!@#$^&*()=|{}':;',\\[\\].<>《》/?~！@#￥……&*（）——|{}【】‘；：”“'。, 、？]",
);
function primaryKeyValidator(_rule: RuleObject, value: string): Promise<void> {
  const combineValue: string = getCombineValueByPrimaryKeys(
    primaryKeys,
    formState,
    props.primaryKeyAvoidRepeatCompareExtra,
  );

  if (
    props.primaryKeyAvoidRepeatCompareTarget
      .map((item): string => item.toLowerCase())
      .includes(combineValue.toLowerCase())
  ) {
    return Promise.reject(new Error(`${combineValue} 已存在`));
  }

  if (SpecialWordsReg.test(value)) {
    return Promise.reject(new Error(`不能含有特殊字符`));
  }

  if (value.toString().includes('_') && !props.primaryKeyUnderline) {
    return Promise.reject(new Error(`不能含有下划线`));
  }

  return Promise.resolve();
}

function noZeroValidator(_rule: RuleObject, value: number): Promise<void> {
  if (Number.isNaN(+value)) {
    return Promise.reject(new Error(`请输入非零数字`));
  }

  if (+value === 0) {
    return Promise.reject(new Error(`请输入非零数字`));
  }

  return Promise.resolve();
}

function instrumnetValidator(_rule: RuleObject, value: string): Promise<void> {
  if (!value) {
    return Promise.resolve();
  }

  const instrumentResolved = transformSearchInstrumentResultToInstrument(value);
  if (!instrumentResolved) {
    return Promise.reject(new Error('标的错误'));
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
    return Promise.reject(new Error('标的错误'));
  }

  return Promise.resolve();
}

function getKfTradeValueName(
  data: Record<number | string | symbol, KungfuApi.KfTradeValueCommonData>,
  key: number | string,
): string {
  return data[key].name;
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
  formState[key] = dayjs(date).format('YYYY-MM-DD HH:mm:ss');
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

function parserPercentString(value: string): string {
  return value.replace('%', '');
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
    :label-col="{ span: labelCol }"
    :wrapper-col="{ span: wrapperCol }"
    :labelAlign="labelAlign"
    :colon="false"
    :scrollToFirstError="true"
    :layout="layout"
  >
    <a-form-item
      v-for="item in configSettings"
      :key="item.key"
      :label="item.name"
      :name="item.key"
      :extra="item.tip"
      :rules="
        changeType === 'update' && item.primary
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
                      message: item.errMsg || '该项为必填项',
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

              ...(item.type === 'instruments'
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
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      ></a-input>
      <a-input-password
        v-else-if="item.type === 'password'"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      ></a-input-password>
      <a-input-number
        v-else-if="item.type === 'int'"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      ></a-input-number>
      <a-input-number
        v-else-if="item.type === 'float'"
        :precision="4"
        step="0.0001"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      ></a-input-number>
      <a-input-number
        v-else-if="item.type === 'percent'"
        :precision="2"
        step="0.01"
        :formatter="formatterPercentNumber"
        :parser="parserPercentString"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      ></a-input-number>
      <a-radio-group
        v-else-if="item.type === 'side'"
        v-model:value="formState[item.key]"
        :name="item.key"
        :disabled="changeType === 'update' && item.primary"
      >
        <a-radio v-for="key in Object.keys(Side).slice(0, 2)" :value="+key">
          {{ Side[(+key) as SideEnum ].name }}
        </a-radio>
      </a-radio-group>
      <a-select
        v-else-if="item.type === 'priceType'"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
      >
        <a-select-option
          v-for="key in Object.keys(PriceType).slice(0, 7)"
          :key="key"
          :value="+key"
        >
          {{ PriceType[+key as PriceTypeEnum].name }}
        </a-select-option>
      </a-select>
      <a-radio-group
        v-else-if="numberEnumRadioType[item.type]"
        v-model:value="formState[item.key]"
        :name="item.key"
        :disabled="changeType === 'update' && item.primary"
      >
        <a-radio
          v-for="key in Object.keys(numberEnumRadioType[item.type])"
          :value="+key"
        >
          {{ getKfTradeValueName(numberEnumRadioType[item.type], key) }}
        </a-radio>
      </a-radio-group>
      <a-select
        v-else-if="numberEnumSelectType[item.type]"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
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
        :disabled="changeType === 'update' && item.primary"
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
        :disabled="changeType === 'update' && item.primary"
      >
        <a-select-option
          v-for="option in item.options"
          :key="option.value"
          :value="option.value"
        >
          {{ option.label }}
        </a-select-option>
      </a-select>
      <a-select
        v-else-if="item.type === 'instrument'"
        :disabled="changeType === 'update' && item.primary"
        show-search
        v-model:value="formState[item.key]"
        :filter-option="false"
        :options="
          instrumentsSearchRelated[item.key].searchInstrumnetOptions.value
        "
        @search="instrumentsSearchRelated[item.key].handleSearchInstrument"
      ></a-select>
      <a-select
        v-else-if="item.type === 'instruments'"
        :disabled="changeType === 'update' && item.primary"
        mode="multiple"
        show-search
        :value="formState[item.key]"
        :filter-option="false"
        :options="
          instrumentsSearchRelated[item.key].searchInstrumnetOptions.value
        "
        @search="instrumentsSearchRelated[item.key].handleSearchInstrument"
        @select="handleInstrumentSelected($event, item.key)"
        @deselect="handleInstrumentDeselected($event, item.key)"
      ></a-select>
      <a-select
        v-else-if="item.type === 'td'"
        v-model:value="formState[item.key]"
        :disabled="changeType === 'update' && item.primary"
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
        :disabled="changeType === 'update' && item.primary"
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
        :disabled="changeType === 'update' && item.primary"
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
      ></a-switch>
      <div
        v-else-if="item.type === 'file'"
        class="kf-form-item__warp file"
        :disabled="changeType === 'update' && item.primary"
      >
        <a-button size="small" @click="handleSelectFile(item.key)">
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
        v-else-if="item.type === 'files'"
        class="kf-form-item__warp file"
        :disabled="changeType === 'update' && item.primary"
      >
        <a-button size="small" @click="handleSelectFiles(item.key)">
          <template #icon><DashOutlined /></template>
        </a-button>
        <div
          v-if="formState[item.key]"
          class="file-path"
          v-for="file in formState[item.key] as string[] || []"
          :title="file"
        >
          <span class="name">{{ file }}</span>
          <close-outlined
            v-if="!(item.default as string[]).includes(file)"
            class="kf-hover"
            @click="handleRemoveFile(item.key, file)"
          />
        </div>
      </div>
      <a-time-picker
        v-else-if="item.type === 'timePicker'"
        :disabled="changeType === 'update' && item.primary"
        :value="dayjs(+formState[item.key] || dayjs())"
        @change="handleTimePickerChange($event as unknown as Dayjs, item.key)"
      ></a-time-picker>
    </a-form-item>
  </a-form>
</template>
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
  }

  .ant-form-item-control-input-content {
    text-align: left;
  }

  .ant-radio-group {
    .ant-radio-wrapper {
      line-height: 2;
    }
  }
}
</style>
