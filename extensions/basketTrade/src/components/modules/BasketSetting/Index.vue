<template>
  <div class="kf-basket-trade-setting__warp kf-translateZ">
    <KfDashboard @board-size-change="handleBodySizeChange">
      <template #header>
        <!-- <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem> -->
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetBasketModal('add')"
          >
            {{ $t('BasketTrade.add_basket') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <a-table
        v-if="basketDataLoaded"
        class="kf-ant-table"
        :columns="columns"
        :data-source="basketsResolved"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :row-class-name="dealBasketRowClassName"
        :custom-row="
          (record) => ({
            onClick: () => {
              setCurrentGlobalBasket(record);
            },
          })
        "
        :default-expand-all-rows="true"
        :empty-text="$t('empty_text')"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.BasketResolved,
          }"
        >
          <template v-if="column.dataIndex === 'marked_value'">
            <KfBlinkNum
              :num="dealAssetPrice(getBasketMarkedValue(record))"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="handleOpenSetBasketModal('update', record)"
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveBasket(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
    <KfSetByConfigModal
      v-if="addBasketModalVisble"
      v-model:visible="addBasketModalVisble"
      :payload="setBasketConfigPayload"
      :primary-key-avoid-repeat-compare-target="basketsNames"
      @confirm="({ formState }) => handleConfirmAddUpdateBasket(formState)"
    ></KfSetByConfigModal>
  </div>
</template>

<script lang="ts" setup>
import { ref, computed, onMounted, toRaw, getCurrentInstance } from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import { SettingOutlined, DeleteOutlined } from '@ant-design/icons-vue';

import {
  messagePrompt,
  useDashboardBodySize,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import {
  buildBasketMapKeyAndLocation,
  dealBasketsToMap,
  getAllBaskets,
  setAllBaskets,
  useBasketMarkedValue,
  useCurrentGlobalBasket,
} from '../../../utils/basketTradeUtils';
import {
  getColumns,
  getBasketFormSettings,
  basketTradingDataGetter,
} from './config';
import { BASKET_CATEGORYS } from '../../../config';
import { DealTradingDataHooks } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { useBasketTradeStore } from '../../../store';
import { dealAssetPrice } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { BasketVolumeTypeEnum } from '@kungfu-trader/kungfu-js-api/src/typings/enums';
const { t } = VueI18n.global;

(
  globalThis.HookKeeper.getHooks().dealTradingData as DealTradingDataHooks
).register(
  {
    category: BASKET_CATEGORYS.SETTING,
    group: '*',
    name: '*',
  } as KungfuApi.KfExtraLocation,
  basketTradingDataGetter,
);

const app = getCurrentInstance();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { currentGlobalBasket, dealBasketRowClassName, setCurrentGlobalBasket } =
  useCurrentGlobalBasket();
const { getBasketMarkedValue } = useBasketMarkedValue();

const basketDataLoaded = ref(false);
const columns = getColumns(getBasketMarkedValue);

const basketsResolvedMap = ref<Record<string, KungfuApi.BasketResolved>>({});
const basketsResolved = computed(() => Object.values(basketsResolvedMap.value));
const addBasketModalVisble = ref(false);
const basketsNames = computed(() => {
  return basketsResolved.value.map((basket) => basket.name);
});

const setBasketConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('BasketTrade.basket'),
  config: {} as KungfuApi.KfExtConfig,
});

const setDefaultGlobalBasket = () => {
  if (!currentGlobalBasket.value) {
    if (basketsResolved.value.length) {
      setCurrentGlobalBasket(basketsResolved.value[0]);
    }
  } else {
    const curBasketKey = buildBasketMapKeyAndLocation(
      currentGlobalBasket.value,
    ).key;
    if (!(curBasketKey in basketsResolvedMap.value)) {
      setCurrentGlobalBasket(
        basketsResolved.value.length ? basketsResolved.value[0] : null,
      );
    }
  }
};

onMounted(() => {
  handleGetAllBaskets().then(() => {
    basketDataLoaded.value = true;
  });

  app?.proxy?.$globalBus.subscribe((data) => {
    if (
      data.tag === 'input:currentConfigModal' &&
      data.category === BASKET_CATEGORYS.SETTING
    ) {
      handleFormStateChange(data.formState);
    }
  });
});

function handleGetAllBaskets() {
  return getAllBaskets()
    .then((baskets) => {
      basketsResolvedMap.value = dealBasketsToMap(baskets);
      useBasketTradeStore().setBasketsMap(basketsResolvedMap.value);
    })
    .then(() => {
      setDefaultGlobalBasket();
    });
}

function handleSetAllBaskets(baskets: KungfuApi.Basket[]) {
  return setAllBaskets(baskets)
    .then((res) => {
      if (res) {
        return handleGetAllBaskets();
      } else {
        return Promise.reject();
      }
    })
    .then(() => {
      messagePrompt().success();
    })
    .catch((err) => {
      if (err instanceof Error) {
        err.message && console.error(err);
      }
      messagePrompt().error();
    });
}

function handleOpenSetBasketModal(
  type: 'add' | 'update',
  initValue?: KungfuApi.Basket,
) {
  setBasketConfigPayload.value.type = type;
  setBasketConfigPayload.value.config = {
    type: [],
    name: t('BasketTrade.basket'),
    category: BASKET_CATEGORYS.SETTING,
    key: BASKET_CATEGORYS.SETTING,
    extPath: '',
    settings: getBasketFormSettings(BasketVolumeTypeEnum.Quantity),
  };

  setBasketConfigPayload.value.initValue =
    type === 'add'
      ? undefined
      : {
          ...initValue,
          total_volume: `${initValue?.total_volume}`,
          volume_type: `${initValue?.volume_type}`,
        };

  addBasketModalVisble.value = true;
}

function handleFormStateChange(formState) {
  setBasketConfigPayload.value.config.settings = getBasketFormSettings(
    +formState.volume_type,
  );
}

function handleConfirmAddUpdateBasket(
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  const newBasket: KungfuApi.Basket = {
    id: new Date().getTime(),
    name: formState.name,
    volume_type: +formState.volume_type,
    total_volume:
      BasketVolumeTypeEnum.Quantity === +formState.volume_type
        ? 0n
        : BigInt(formState.total_volume),
  };

  const newBasketMap = dealBasketsToMap([newBasket]);
  const allBaskets = Object.assign(
    toRaw(basketsResolvedMap.value),
    newBasketMap,
  );

  return handleSetAllBaskets(Object.values(allBaskets));
}

function handleRemoveBasket(targetBasket: KungfuApi.BasketResolved) {
  const allBaskets = Object.assign({}, toRaw(basketsResolvedMap.value));
  delete allBaskets[targetBasket.location_id];

  return handleSetAllBaskets(Object.values(allBaskets));
}
</script>

<style lang="less">
.kf-basket-trade-setting__warp {
  height: 100%;
}
</style>
