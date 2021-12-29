<script setup lang="ts">
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@renderer/components/public/KfBlinkNum.vue';

import {
    useDashboardBodySize,
    useExtConfigsRelated,
    useInstruments,
    useProcessStatusDetailData,
    useQuote,
    useTriggerOrderBook,
} from '@renderer/assets/methods/uiUtils';
import { computed, getCurrentInstance, nextTick, ref, watch } from 'vue';
import { getColumns } from './config';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    addSubscribeInstruments,
    removeSubscribeInstruments,
} from '@renderer/assets/methods/actionsUtils';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings';
import { StarFilled } from '@ant-design/icons-vue';
import { message } from 'ant-design-vue';
import {
    dealAssetPrice,
    dealKfNumber,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

interface MarketDataProps {}
defineProps<MarketDataProps>();

const app = getCurrentInstance();
const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
    useDashboardBodySize();

const columns = computed(() => {
    if (dashboardBodyWidth.value < 450) {
        return getColumns(true);
    }
    return getColumns(false);
});

const {
    instruments,
    subscribedInstruments,
    subscribeAllInstrumentByAppStates,
} = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap } = useExtConfigsRelated();

const options = ref<{ value: string; label: string }[]>([]);
const searchResults = ref<string[]>([]);
const { getQuoteByInstrument, getLastPricePercent } = useQuote();
const { customRow } = useTriggerOrderBook();

function handleSearchInstruments(val: string) {
    options.value = instruments.value
        .filter((item) => {
            return item.id.includes(val);
        })
        .slice(0, 20)
        .map((item) => ({
            value: `${item.exchangeId}_${item.instrumentId}_${item.instrumentType}_${item.ukey}_${item.instrumentName}`,
            label: `${item.instrumentId} ${item.instrumentName} ${
                ExchangeIds[item.exchangeId.toUpperCase()].name
            }`,
        }));
}

function handleSubscribeAll() {
    subscribeAllInstrumentByAppStates(
        processStatusData.value,
        appStates.value,
        mdExtTypeMap.value,
        subscribedInstruments.value,
    );
    message.success('操作成功');
}

function handleConfirmAddInstrument(val: string[]): Promise<void> {
    nextTick().then(() => {
        searchResults.value = [];
    });

    const instruments: InstrumentResolved[] = val
        .map((ins): InstrumentResolved | null => {
            const pair = ins.split('_');
            if (pair.length !== 5) return null;
            const [
                exchangeId,
                instrumentId,
                instrumentType,
                ukey,
                instrumentName,
            ] = pair;
            return {
                exchangeId,
                instrumentId,
                instrumentType: +instrumentType as InstrumentTypeEnum,
                instrumentName,
                id: `${instrumentId}_${instrumentName}_${exchangeId}`.toLowerCase(),
                ukey,
            };
        })
        .filter((ins) => !!ins) as InstrumentResolved[];

    return addSubscribeInstruments(instruments)
        .then(() => {
            if (app?.proxy) {
                app.proxy.$useGlobalStore().setSubscribedInstruments();
            }
        })
        .then(() => {
            subscribeAllInstrumentByAppStates(
                processStatusData.value,
                appStates.value,
                mdExtTypeMap.value,
                instruments,
            );
        })
        .then(() => {
            message.success('操作成功');
        });
}

function handleConfirmRemoveInstrument(instrument: InstrumentResolved) {
    return removeSubscribeInstruments(instrument)
        .then(() => {
            if (app?.proxy) {
                app.proxy.$useGlobalStore().setSubscribedInstruments();
            }
        })
        .then(() => {
            message.success('操作成功');
        })
        .catch((err) => {
            message.error(err.message || '操作失败');
        });
}
</script>
<template>
    <div class="kf-market-data__warp">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-select
                        show-search
                        v-model:value="searchResults"
                        mode="multiple"
                        :autoClearSearchValue="true"
                        placeholder="添加自选"
                        style="width: 200px"
                        :filter-option="false"
                        :options="options"
                        @search="handleSearchInstruments"
                        @change="handleConfirmAddInstrument"
                    ></a-select>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button size="small" @click="handleSubscribeAll">
                        订阅
                    </a-button>
                </KfDashboardItem>
            </template>
            <a-table
                :columns="columns"
                :data-source="subscribedInstruments.value"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
                :customRow="customRow"
                emptyText="暂无数据"
            >
                <template
                    #bodyCell="{
                        column,
                        record,
                    }: {
                        column: AntTableColumn,
                        record: InstrumentResolved,
                    }"
                >
                    <template v-if="column.dataIndex === 'instrumentId'">
                        <div class="subscribed-instrument-info">
                            <div class="name">{{ record.instrumentName }}</div>
                            <div class="code">
                                <span>{{ record.instrumentId }}</span>
                                <span>
                                    {{ ExchangeIds[record.exchangeId].name }}
                                </span>
                            </div>
                        </div>
                    </template>
                    <template v-else-if="column.dataIndex === 'open_price'">
                        {{
                            dealAssetPrice(
                                getQuoteByInstrument(record)?.open_price,
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'low_price'">
                        {{
                            dealAssetPrice(
                                getQuoteByInstrument(record)?.low_price,
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'high_price'">
                        {{
                            dealAssetPrice(
                                getQuoteByInstrument(record)?.high_price,
                            )
                        }}
                    </template>

                    <template v-else-if="column.dataIndex === 'lastPrice'">
                        <div class="last-price-content">
                            <div class="price">
                                {{
                                    dealAssetPrice(
                                        getQuoteByInstrument(record)
                                            ?.last_price,
                                    )
                                }}
                            </div>
                            <div class="percent">
                                <KfBlinkNum
                                    blink-type="color"
                                    mode="compare-zero"
                                    :num="getLastPricePercent(record)"
                                ></KfBlinkNum>
                            </div>
                        </div>
                    </template>
                    <template v-else-if="column.dataIndex === 'volume'">
                        {{ dealKfNumber(getQuoteByInstrument(record)?.volume) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <StarFilled
                                style="color: #faad14"
                                @click="handleConfirmRemoveInstrument(record)"
                            />
                        </div>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
    </div>
</template>
<script lang="ts">
export default {
    name: '行情订阅',
};
</script>
<style lang="less">
.kf-market-data__warp {
    height: 100%;

    .subscribed-instrument-info {
        .name {
            font-size: 12px;
            font-weight: bold;
        }

        .code {
            span {
                font-size: 10px;
                padding-right: 6px;
            }
        }
    }

    .last-price-content {
        .price {
            font-size: 12px;
        }

        .percent {
            font-size: 10px;
        }

        .percent {
            position: relative;
            height: 18px;
            width: 100%;

            .kf-blink-num {
                padding: 0;
            }
        }
    }
}
</style>
