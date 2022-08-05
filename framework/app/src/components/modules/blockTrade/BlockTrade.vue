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
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useCurrentGlobalKfLocation,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  confirmModal,
  messagePrompt,
  useDashboardBodySize,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { dealOrderPlaceVNode } from './utils';

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

const configSettings = computed(() => {
  if (!currentGlobalKfLocation.value) {
    return getConfigSettings();
  }

  const { category } = currentGlobalKfLocation.value;
  return getConfigSettings(category);
});

function numberValidator(_rule: RuleObject, value: string) {
  if (value === '' || !/^\d+$/.test(value)) {
    return Promise.reject(t('blockTradeConfig.only_number'));
  } else {
    return Promise.resolve();
  }
}

const rules = {
  opponent_seat: {
    validator: numberValidator,
    trigger: 'change',
  },
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
        hedge_flag,
        is_swap,
        opponent_seat,
        opponent_account,
        match_number,
        linkman,
        contact_way,
        underweight_type,
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
        is_swap: !!is_swap,
      };

      const blockMessage: KungfuApi.BlockMessage = {
        opponent_seat: +opponent_seat || 0,
        opponent_account: opponent_account || '',
        match_number: match_number || '',
        value: {
          linkman: linkman || '',
          contact_way: contact_way || '',
          underweight_type: +underweight_type,
        },
        insert_time: 0n,
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

      await confirmModal(
        t('tradingConfig.place_confirm'),
        dealOrderPlaceVNode({ ...makeOrderInput, ...blockMessage }, 1),
      );

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
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
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
            :configSettings="configSettings"
            changeType="add"
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
