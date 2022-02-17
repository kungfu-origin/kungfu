<script setup lang="ts">
import {
  computed,
  getCurrentInstance,
  nextTick,
  onBeforeUnmount,
  onMounted,
  ref,
  watch,
} from 'vue';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfConfigSettingsForm from '@renderer/components/public/KfConfigSettingsForm.vue';
import {
  buildInstrumentSelectOptionValue,
  initFormStateByConfig,
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useProcessStatusDetailData,
  useTriggerMakeOrder,
} from '@renderer/assets/methods/uiUtils';
import { getConfigSettings } from './config';
import { message } from 'ant-design-vue';
import { makeOrderByOrderInput } from '@kungfu-trader/kungfu-js-api/kungfu';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useInstruments } from '@renderer/assets/methods/actionsUtils';
import {
  getProcessIdByKfLocation,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

interface MakeOrderProps {}
defineProps<MakeOrderProps>();

const app = getCurrentInstance();
const formState = ref(
  initFormStateByConfig(getConfigSettings('td', InstrumentTypeEnum.future), {}),
);
const formRef = ref();
const { subscribeAllInstrumentByAppStates } = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap } = useExtConfigsRelated();
const { triggerOrderBook } = useTriggerMakeOrder();

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const makeOrderInstrumentType = ref<InstrumentTypeEnum>(
  InstrumentTypeEnum.unknown,
);

const configSettings = computed(() => {
  if (!currentGlobalKfLocation.data) {
    return getConfigSettings();
  }

  const { category } = currentGlobalKfLocation.data;
  return getConfigSettings(
    category,
    makeOrderInstrumentType.value,
    +formState.value.price_type,
  );
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$bus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'makeOrder') {
        const { offset, side, volume, price, instrumentType, accountId } = (
          data as TriggerMakeOrder
        ).orderInput;

        const instrumentValue = buildInstrumentSelectOptionValue(
          (data as TriggerMakeOrder).orderInput,
        );

        formState.value.instrument = instrumentValue;
        formState.value.offset = +offset;
        formState.value.side = +side;
        formState.value.volume = +Number(volume).toFixed(0);
        formState.value.price = +Number(price).toFixed(4);
        formState.value.instrument_type = +instrumentType;

        if (accountId) {
          formState.value.account_id = accountId;
        }
      }

      if (data.tag === 'orderBookUpdate') {
        const { side, price, volume, instrumentType } = (
          data as TriggerOrderBookUpdate
        ).orderInput;

        const instrumentValue = buildInstrumentSelectOptionValue(
          (data as TriggerOrderBookUpdate).orderInput,
        );

        if (!formState.value.instrument) {
          formState.value.instrument = instrumentValue;
          formState.value.instrument_type = +instrumentType;
        }
        if (+price !== 0) {
          formState.value.price = +Number(price).toFixed(4);
        }
        if (BigInt(volume) !== BigInt(0)) {
          formState.value.volume = +Number(volume).toFixed(0);
        }

        formState.value.side = +side;
      }
    });

    onBeforeUnmount(() => {
      subscription.unsubscribe;
    });
  }
});

watch(
  () => formState.value.instrument,
  (newVal) => {
    const instrumentResolved = transformSearchInstrumentResultToInstrument(
      newVal.toString(),
    );

    if (!instrumentResolved) {
      return;
    }

    subscribeAllInstrumentByAppStates(
      processStatusData.value,
      appStates.value,
      mdExtTypeMap.value,
      [instrumentResolved],
    );

    triggerOrderBook(instrumentResolved);
    makeOrderInstrumentType.value = instrumentResolved.instrumentType;
  },
);

function handleResetMakeOrderForm() {
  const initFormState = initFormStateByConfig(configSettings.value, {});
  Object.keys(initFormState).forEach((key) => {
    formState.value[key] = initFormState[key];
  });
  nextTick().then(() => {
    formRef.value.clearValidate();
  });
}

function handleMakeOrder() {
  formRef.value
    .validate()
    .then(() => {
      const instrument = formState.value.instrument.toString();
      const instrumnetResolved =
        transformSearchInstrumentResultToInstrument(instrument);

      if (!instrumnetResolved) {
        message.error('标的错误');
        return;
      }

      const { exchangeId, instrumentId, instrumentType } = instrumnetResolved;

      const {
        account_id,
        limit_price,
        volume,
        price_type,
        side,
        offset,
        hedge_flag,
      } = formState.value;

      const makeOrderInput: KungfuApi.MakeOrderInput = {
        instrument_id: instrumentId,
        instrument_type: +instrumentType,
        exchange_id: exchangeId,
        limit_price: +limit_price,
        volume: +volume,
        price_type: +price_type,
        side: +side,
        offset: +(offset !== undefined ? offset : +side === 0 ? 0 : 1),
        hedge_flag: +(hedge_flag || 0),
      };

      if (!currentGlobalKfLocation.data || !window.watcher) {
        message.error('当前 Location 错误');
        return;
      }

      const tdProcessId =
        currentGlobalKfLocation.data.category === 'td'
          ? getProcessIdByKfLocation(currentGlobalKfLocation.data)
          : `td_${account_id.toString()}`;

      if (processStatusData.value[tdProcessId] !== 'online') {
        message.error(`请先启动 ${tdProcessId} 交易进程`);
        return;
      }

      makeOrderByOrderInput(
        window.watcher,
        makeOrderInput,
        currentGlobalKfLocation.data,
        account_id.toString(),
      ).catch((err) => {
        message.error(err.message);
      });
    })
    .catch((err: Error) => {
      console.error(err);
    });
}
</script>

<template>
  <div class="kf-make-order-dashboard__warp">
    <KfDashboard>
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation.data">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation.data">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation.data) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-button size="small" @click="handleResetMakeOrderForm">
            重置
          </a-button>
        </KfDashboardItem>
      </template>
      <div class="make-order__warp">
        <div class="make-order-form__warp">
          <KfConfigSettingsForm
            ref="formRef"
            v-model:formState="formState"
            :configSettings="configSettings"
            changeType="add"
            :label-col="5"
            :wrapper-col="14"
          ></KfConfigSettingsForm>
        </div>
        <div class="make-order-btns">
          <a-button size="small" @click="handleMakeOrder">下单</a-button>
        </div>
      </div>
    </KfDashboard>
  </div>
</template>
<style lang="less">
.kf-make-order-dashboard__warp {
  width: 100%;
  height: 100%;

  .make-order__warp {
    display: flex;
    justify-content: space-between;
    height: 100%;

    .make-order-form__warp {
      flex: 1;
      height: 100%;
      padding-top: 16px;
      overflow-y: overlay;

      .ant-form-item {
        margin-bottom: 16px;

        .ant-form-item-explain,
        .ant-form-item-extra {
          min-height: unset;
        }
      }
    }

    .make-order-btns {
      width: 40px;
      height: 100%;

      .ant-btn {
        height: 100%;
        text-align: center;
        word-break: break-word;
        word-wrap: unset;
        white-space: normal;
      }
    }
  }
}
</style>
