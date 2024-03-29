<script setup lang="ts">
import { computed, nextTick, ref } from 'vue';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfConfigSettingsForm from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfConfigSettingsForm.vue';
import { getConfigSettings } from './config';
import { RuleObject } from 'ant-design-vue/lib/form';
import { makeOrderByBlockMessage } from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  getProcessIdByKfLocation,
  initFormStateByConfig,
  isShotable,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useCurrentGlobalKfLocation,
  useMakeOrderSubscribe,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  confirmModal,
  messagePrompt,
  useDashboardBodySize,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { dealOrderPlaceVNode } from './utils';
import {
  HedgeFlagEnum,
  InstrumentTypeEnum,
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

const { t } = VueI18n.global;
const { error } = messagePrompt();

const { handleBodySizeChange } = useDashboardBodySize();

const formState = ref(initFormStateByConfig(getConfigSettings(), {}));
const formRef = ref();
const { processStatusData } = useProcessStatusDetailData();

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);
useMakeOrderSubscribe(formState);

const getResolvedOffset = (
  offset: OffsetEnum,
  side: SideEnum,
  instrumentType: InstrumentTypeEnum,
) => {
  if (isShotable(instrumentType)) {
    if (offset !== undefined) {
      return offset;
    }
  }
  return side === 0 ? 0 : 1;
};

const configSettings = computed(() => {
  if (!currentGlobalKfLocation.value) {
    return getConfigSettings();
  }

  const { category } = currentGlobalKfLocation.value;
  return getConfigSettings(category);
});

function numberValidator(_rule: RuleObject, value: string | number) {
  if (value === '' || !/^\d+$/.test(value + '')) {
    return Promise.reject(t('blockTradeConfig.only_number'));
  } else {
    return Promise.resolve();
  }
}

const rules = {
  match_number: {
    validator: numberValidator,
    trigger: 'change',
  },
};

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
    .then(async () => {
      const instrument = formState.value.instrument.toString();
      const instrumnetResolved =
        transformSearchInstrumentResultToInstrument(instrument);

      if (!instrumnetResolved) {
        error(t('instrument_error'));
        return;
      }

      const { exchangeId, instrumentType, instrumentId } = instrumnetResolved;

      const {
        account_id,
        limit_price,
        volume,
        price_type,
        side,
        offset,
        is_swap,
        opponent_seat,
        match_number,
        is_specific,
      } = formState.value;

      const makeOrderInput: KungfuApi.MakeOrderInput = {
        instrument_id: instrumentId,
        instrument_type: +instrumentType,
        exchange_id: exchangeId,
        limit_price: +limit_price,
        volume: +volume,
        price_type: +price_type,
        side: +side,
        offset: getResolvedOffset(+offset, +side, +instrumentType),
        hedge_flag: HedgeFlagEnum.Speculation,
        is_swap: !!is_swap,
        parent_id: 0n,
      };

      const blockMessage: KungfuApi.BlockMessage = {
        opponent_seat: opponent_seat || '',
        match_number: match_number || '',
        is_specific: !!is_specific,
        insert_time: 0n,
        block_id: 0n,
      };

      if (!currentGlobalKfLocation.value) {
        error(t('location_error'));
        return;
      }

      const tdProcessId =
        currentGlobalKfLocation.value?.category === 'td'
          ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
          : `td_${account_id.toString()}`;

      if (processStatusData.value[tdProcessId] !== 'online') {
        error(
          `${t('orderConfig.start')} ${tdProcessId} ${t('orderConfig.td')}`,
        );
        return;
      }

      const flag = await confirmModal(
        t('tradingConfig.place_confirm'),
        dealOrderPlaceVNode({ ...makeOrderInput, ...blockMessage }, 1),
      );
      if (!flag) return;

      makeOrderByBlockMessage(
        window.watcher,
        blockMessage,
        makeOrderInput,
        currentGlobalKfLocation.value,
        tdProcessId.toAccountId(),
      ).catch((err) => {
        error(err.message);
      });
    })
    .catch((err: Error) => {
      console.error(err);
    });
}
</script>
<template>
  <div class="kf-make-order-dashboard__warp">
    <KfDashboard @board-size-change="handleBodySizeChange">
      <template #title>
        <span v-if="currentGlobalKfLocation">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span v-if="currentGlobalKfLocation" class="name">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation) }}
          </span>
        </span>
      </template>
      <template #header>
        <KfDashboardItem>
          <a-button size="small" @click="handleResetMakeOrderForm">
            {{ $t('blockTradeConfig.reset_order') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <div class="make-order__warp">
        <div class="make-order-form__warp">
          <KfConfigSettingsForm
            ref="formRef"
            v-model:formState="formState"
            :config-settings="configSettings"
            change-type="add"
            :label-col="6"
            :wrapper-col="14"
            :rules="rules"
          ></KfConfigSettingsForm>
        </div>
        <div class="make-order-btns">
          <a-button size="small" @click="handleMakeOrder">
            {{ $t('blockTradeConfig.place_order') }}
          </a-button>
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
.modal-node {
  .root-node {
    display: flex;
    flex-wrap: nowrap;
    .green {
      color: @green-base !important;
    }
    .red {
      color: @red-base !important;
    }
    .order-number {
      flex: 1;
      margin-top: 10%;
      text-align: center;
    }
  }
}
</style>
