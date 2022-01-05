<script lang="ts" setup>
import { sum } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Empty } from 'ant-design-vue';
import { computed, getCurrentInstance, ref } from 'vue';

const props = withDefaults(
    defineProps<{
        dataSource: TradingDataItem[];
        columns: KfTradingDataTableHeaderConfig[];
        keyField?: string;
    }>(),
    {
        columns: () => [],
        dataSource: () => [],
        keyField: 'id',
    },
);

defineEmits<{
    (e: 'dbclickRow', data: { event: MouseEvent; row: TradingDataItem }): void;
    (e: 'clickRow', data: { event: MouseEvent; row: TradingDataItem }): void;
    (
        e: 'clickCell',
        data: {
            event: MouseEvent;
            row: TradingDataItem;
            column: KfTradingDataTableHeaderConfig;
        },
    ): void;
    (
        e: 'rightClickRow',
        data: { event: MouseEvent; row: TradingDataItem },
    ): void;
}>();

const app = getCurrentInstance();
const simpleImage = Empty.PRESENTED_IMAGE_SIMPLE;
const kfScrollerTableBodyRef = ref();
const kfScrollerTableWidth = ref(0);
let clickTimer: number | undefined;

const headerWidth = computed(() => {
    const widths: KfTradingDataTableHeaderConfig[] = []; //column use with
    const flexs: KfTradingDataTableHeaderConfig[] = []; //column use flex

    props.columns.forEach((item) => {
        if (item.width !== undefined) {
            widths.push(item);
        } else {
            flexs.push(item);
        }
    });

    const flexWidthUnits = sum(flexs.map((item) => item.flex || 1));
    const widthForFlex =
        kfScrollerTableWidth.value -
        (widths.length ? sum(widths.map((item) => item.width || 0)) : 0);
    const unit = widthForFlex / flexWidthUnits;

    let headerWidthCollection: Record<string, string | number> = {};
    [...widths, ...flexs].forEach((item) => {
        headerWidthCollection[item.dataIndex] = item.width
            ? item.width + 'px'
            : unit * (item.flex || 1) + 'px';
    });

    return headerWidthCollection;
});

function getHeaderWidth(column: KfTradingDataTableHeaderConfig) {
    const headerWidthByCalc = headerWidth.value[column.dataIndex];
    const columnWidth = column.width;

    if (headerWidthByCalc <= 0 || !headerWidthByCalc) {
        return columnWidth;
    } else {
        return headerWidthByCalc;
    }
}

function handleClickRow(e: MouseEvent, row: TradingDataItem) {
    app && app.emit('clickRow', { event: e, row });
    clickTimer && clearTimeout(clickTimer);
}

function handleDbClickRow(e: MouseEvent, row: TradingDataItem) {
    app && app.emit('dbclickRow', { event: e, row });
    clickTimer && clearTimeout(clickTimer);
}

function handleMousedown(e: MouseEvent, row: TradingDataItem) {
    if (e.button === 2) {
        app && app.emit('rightClickRow', { event: e, row });
    }
}
</script>
<template>
    <div class="kf-table">
        <ul class="kf-table-header kf-table-row">
            <li
                v-for="column in columns"
                :class="['text-overflow', 'kf-table-cell', column.type]"
                :key="column.dataIndex"
                :title="column.name"
                :style="{
                    'max-width': getHeaderWidth(column),
                }"
            >
                {{ column.name }}
            </li>
        </ul>
        <div class="kf-table-body" ref="kfScrollerTableBodyRef">
            <RecycleScroller
                v-if="dataSource && dataSource.length"
                class="kf-table-scroller"
                :items="dataSource"
                :item-size="26"
                :key-field="keyField"
                :buffer="100"
            >
                <template v-slot="{ item }: { item: any }">
                    <ul
                        class="kf-table-row"
                        @click="handleClickRow($event, item)"
                        @dblclick="handleDbClickRow($event, item)"
                        @mousedown="handleMousedown($event, item)"
                    >
                        <li
                            v-for="column in columns"
                            :title="item[column.dataIndex as keyof TradingDataItem] || ''"
                            :class="[
                                'text-overflow',
                                'kf-table-cell',
                                column.type,
                            ]"
                            :key="`${column.dataIndex}_${item[keyField as keyof TradingDataItem]}`"
                            :style="{
                                'max-width': getHeaderWidth(column),
                            }"
                        >
                            <slot :item="item" :column="column">
                                <span>
                                    {{ item[column.dataIndex as keyof TradingDataItem] }}
                                </span>
                            </slot>
                        </li>
                    </ul>
                </template>
            </RecycleScroller>
            <a-empty v-else :image="simpleImage"></a-empty>
        </div>
    </div>
</template>
<style lang="less">
.kf-table {
    display: flex;
    flex-direction: column;
    height: 100%;
    position: relative;

    .ant-empty {
        height: auto;
        margin-top: 48px;

        .ant-empty-image {
            height: auto;
        }

        .ant-empty-description {
            color: @input-placeholder-color;
        }
    }

    .kf-table-header {
        display: flex;
        flex-direction: row;
        justify-content: flex-start;
        height: 36px;
        line-height: 36px;
        background: @table-header-bg;
        white-space: nowrap;
        box-sizing: border-box;
        margin-bottom: 4px;

        .kf-table-cell {
            height: 36px;
            line-height: 36px;
            display: inline-block;
            user-select: none;
            position: relative;

            &::after {
                content: '';
                width: 0;
                height: 60%;
                position: absolute;
                right: 0;
                top: 50%;
                transform: translateY(-50%);
                border-left: 1px solid @table-header-cell-split-color;
            }
        }
    }

    .kf-table-body {
        width: 100%;
        transform: translateZ(0);
        position: absolute;
        top: 36px;
        height: calc(100% - 36px);

        .kf-table-scroller {
            height: 100%;
            width: 100%;
        }
    }

    .kf-table-row {
        display: flex;
        flex-direction: row;
        justify-content: flex-start;
        cursor: pointer;
    }

    .kf-table-row:hover {
        background: @table-row-hover-bg;
    }

    .kf-table-cell {
        height: 26px;
        line-height: 26px;
        padding: 0 6px;
        box-sizing: border-box;
        word-wrap: break-word;
        flex: 1;
        font-size: 12px;
        user-select: text;
        text-align: left;
        position: relative;

        &.number {
            text-align: right;
        }
    }
}
</style>
