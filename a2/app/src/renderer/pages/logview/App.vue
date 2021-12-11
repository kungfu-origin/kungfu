<script setup lang="ts">
import path from 'path';
import { ref } from 'vue';
import {
    parseURIParams,
    removeLoadingMask,
} from '@renderer/assets/methods/pureUiUtils';
import { LOG_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
    buildListByLineLimit,
    dealLogMessage,
    getLog,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import dayjs from 'dayjs';
import { KfNumListRtn } from '@kungfu-trader/kungfu-js-api/typings';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';

const processId = ref<string>('');

const logList = ref<Array<KfLogData>>([]);
initLog()
    .then((numList: KfNumListRtn<KfLogData>) => {
        logList.value = numList.list;
    })
    .finally(() => {
        removeLoadingMask();
    });

function initLog(): Promise<KfNumListRtn<KfLogData>> {
    processId.value = parseURIParams().processId || '';
    if (processId.value) {
        const logPath = path.resolve(
            LOG_DIR,
            dayjs().format('YYYYMMDD'),
            `${processId.value}.log`,
        );

        document.getElementsByTagName(
            'title',
        )[0].innerText = `功夫交易系统 - ${processId.value}.log`;

        return getLog({
            logPath,
            fromStart: true,
            dealLogLineMethod: dealLogMessage,
        });
    }

    return Promise.resolve(buildListByLineLimit<KfLogData>(3000));
}
</script>
<template>
    <a-layout>
        <div class="kf-log-view__warp">
            <KfDashboard>
                <template v-slot:header>
                    <KfDashboardItem>
                        <a-checkbox
                            size="small"
                            style="margin-right: 8px"
                        ></a-checkbox>
                        <span style="font-size: 12px">滚动到底部</span>
                    </KfDashboardItem>
                    <KfDashboardItem>
                        <a-button size="small">文件夹</a-button>
                    </KfDashboardItem>
                    <KfDashboardItem>
                        <a-button size="small" type="primary">清空</a-button>
                    </KfDashboardItem>
                </template>

                <DynamicScroller
                    class="kf-table"
                    :items="logList"
                    :min-item-size="36"
                >
                    <template
                        v-slot="{
                            item,
                            index,
                            active,
                        }: {
                            item: KfLogData,
                            index: number,
                            active: boolean,
                        }"
                    >
                        <DynamicScrollerItem
                            :item="item"
                            :active="active"
                            :size-dependencies="[item.message]"
                            :data-index="index"
                        >
                            <div
                                class="kf-log-line"
                                v-html="item.message"
                            ></div>
                        </DynamicScrollerItem>
                    </template>
                </DynamicScroller>
            </KfDashboard>
        </div>
    </a-layout>
</template>

<style lang="less">
@import '@renderer/assets/less/base.less';
@import '@renderer/assets/less/public.less';

#app {
    width: 100%;
    height: 100%;
    font-family: Avenir, Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;

    .ant-layout {
        height: 100%;
        background: @component-background;

        .kf-dashboard__body {
            background: #000;
            padding: 8px;
            box-sizing: border-box;
            border-radius: 4px;
        }
    }

    .kf-log-view__warp {
        height: 100%;
        width: 100%;
        padding: 0 8px 8px 8px;
    }

    .kf-log-line {
        text-align: left;
        font-size: 14px;
        user-select: text;
        padding-bottom: 4px;
        line-height: 1.5;

        .error {
            color: @red-6;
            font-weight: bold;
        }

        .debug {
            color: @blue-6;
            font-weight: bold;
        }

        .warning {
            color: @orange-6;
            font-weight: bold;
        }

        .trace {
            color: @green-6;
            font-weight: bold;
        }

        .critical {
            color: @red-6;
            font-weight: bolder;
        }
    }
}
</style>
