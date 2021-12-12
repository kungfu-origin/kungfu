<script setup lang="ts">
import path from 'path';
import { Tail } from 'tail';
import {
    computed,
    nextTick,
    onMounted,
    reactive,
    ref,
    toRef,
    watch,
} from 'vue';
import { UpOutlined, DownOutlined } from '@ant-design/icons-vue';

import {
    parseURIParams,
    removeLoadingMask,
} from '@renderer/assets/methods/uiUtils';
import { LOG_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
    debounce,
    getLog,
    KfNumList,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import dayjs from 'dayjs';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import { ensureFileSync, outputFile } from 'fs-extra';
import { message } from 'ant-design-vue';
import { shell } from '@electron/remote';
import { clipboard } from 'electron';
import { platform } from 'os';

onMounted(() => {
    removeLoadingMask();
    cleanSearchRelatedState();
});

const logList = reactive<KfNumList<KfLogData>>(new KfNumList(2000));
const processId = ref<string>('');
const initLogLoading = ref<boolean>(false);
const logPath = ref<string>('');
const scrollerTableRef = ref();
const inputSearchRef = ref();
var watchLogTail: Tail | null = null;

initLogLoading.value = true;
initLog()
    .then((numList: KfNumList<KfLogData>) => {
        logList.list = numList.list;
    })
    .finally(() => {
        initLogLoading.value = false;
        startTailFile(logPath.value);
    });

function initLog(): Promise<KfNumList<KfLogData>> {
    processId.value = parseURIParams().processId || '';
    if (processId.value) {
        logPath.value = path.resolve(
            LOG_DIR,
            dayjs().format('YYYYMMDD'),
            `${processId.value}.log`,
        );

        document.getElementsByTagName(
            'title',
        )[0].innerText = `功夫交易系统 - ${processId.value}.log`;

        ensureFileSync(logPath.value);
        return getLog({
            logPath: logPath.value,
            dealLogLineMethod: dealLogMessage,
        });
    }

    return Promise.resolve(new KfNumList<KfLogData>(2000));
}

function startTailFile(logPath: string) {
    ensureFileSync(logPath);
    watchLogTail && watchLogTail.unwatch();
    watchLogTail = new Tail(logPath, {
        follow: true,
    });
    let newId = +new Date();
    watchLogTail.on('line', (data) => {
        logList.insert({
            id: newId++,
            message: dealLogMessage(data),
            messageOrigin: data,
            messageForSearch: '',
        });
        handleSwitchScrollToBottomChecked();
    });
    watchLogTail.watch();
}

const searchKeyword = ref<string>('');
const searchKeywordReg = computed(() => {
    let reg = null;
    try {
        reg = new RegExp(searchKeyword.value, 'g');
    } catch (err) {
        console.error(err);
    }

    return reg;
});
const currentResultPointerIndex = ref<number>(0);
const totalResultCount = ref<number>(0);
var searchResultList: KfLogData[] = [];

watch(
    searchKeywordReg,
    debounce(() => {
        //clean
        if (
            searchKeyword.value.trim() === '' ||
            searchKeywordReg.value === null
        ) {
            cleanSearchRelatedState();
            return;
        }

        let isFirst = true;
        searchResultList = [];
        logList.list.forEach((item: KfLogData) => {
            if (item.message.indexOf(searchKeyword.value) !== -1) {
                item.messageForSearch = setLogDataMessageForSearch(item);

                if (isFirst) {
                    item.messageForSearch = setLogDataMessageForSearch(
                        item,
                        true,
                    );
                    isFirst = false;
                }

                searchResultList.push(item);
            } else {
                item.messageForSearch = '';
            }
        });

        const total = searchResultList.length;
        totalResultCount.value = total;
        if (total > 0) {
            currentResultPointerIndex.value = 1;
        } else {
            currentResultPointerIndex.value = 0;
        }
    }),
);

watch(currentResultPointerIndex, (newIndex: number, oldIndex: number) => {
    if (newIndex === 0) {
        return;
    }

    const oldId = oldIndex ? searchResultList[oldIndex - 1].id : 0;
    const newId = searchResultList[newIndex - 1].id;

    logList.list.forEach((logData: KfLogData, index: number) => {
        if (logData.id === oldId && oldId !== 0) {
            logData.messageForSearch = setLogDataMessageForSearch(logData);
        }

        if (logData.id === newId) {
            logData.messageForSearch = setLogDataMessageForSearch(
                logData,
                true,
            );
            srollToItemByIndexInLogList(index);
        }
    });
});

document.addEventListener('keydown', (e) => {
    const ctrlCmd = platform() === 'darwin' ? e.metaKey : e.ctrlKey;
    if (ctrlCmd && e.key === 'f') {
        searchKeyword.value = clipboard.readText();
        if (inputSearchRef.value) {
            const $input = inputSearchRef.value.$el.firstElementChild;
            if ($input) {
                $input.focus();
                nextTick().then(() => {
                    $input.select();
                });
            }
        }
    }
});

function setLogDataMessageForSearch(
    item: KfLogData,
    currentPointer = false,
): string {
    if (searchKeywordReg.value === null) return '';
    if (currentPointer) {
        return dealLogMessage(
            item.messageOrigin.replace(
                searchKeywordReg.value,
                `<font class="search-keyword current-search-pointer">${searchKeyword.value}</font>`,
            ),
        );
    } else {
        return dealLogMessage(
            item.messageOrigin.replace(
                searchKeywordReg.value,
                `<font class="search-keyword">${searchKeyword.value}</font>`,
            ),
        );
    }
}

function srollToItemByIndexInLogList(index: number): void {
    const id = logList.list[index].id;
    const $item = document.getElementById(`log-item-${id}`);
    //vue-virtual-scroller 的scrollToItem方法有问题，以这种方法让搜索结果显示出来
    const indexResolved = index < 10 ? index : index - 5;

    if ($item) {
        const rect = $item.getBoundingClientRect();
        if (rect.top < 0) {
            scrollerTableRef.value.scrollToItem(indexResolved);
        } else {
            const tableClientHeight =
                document.getElementById('kf-log-table')?.clientHeight || 0;
            if (tableClientHeight && tableClientHeight < rect.top) {
                scrollerTableRef.value.scrollToItem(indexResolved);
            }
        }
    } else {
        scrollerTableRef.value.scrollToItem(indexResolved);
    }
}

const scrollToBottomChecked = ref<boolean>(false);
function handleSwitchScrollToBottomChecked() {
    if (scrollToBottomChecked.value) {
        scrollerTableRef.value.scrollToBottom(303);
    }
}

function handleToDownSearchResult(): void {
    if (totalResultCount.value === 0) return;
    if (currentResultPointerIndex.value === totalResultCount.value) {
        currentResultPointerIndex.value = 1;
    } else {
        currentResultPointerIndex.value++;
    }
}

function handleToUpSearchResult(): void {
    if (totalResultCount.value === 0) return;
    if (currentResultPointerIndex.value === 1) {
        currentResultPointerIndex.value = totalResultCount.value;
    } else {
        currentResultPointerIndex.value--;
    }
}

function handleRemoveLog(): Promise<void> {
    ensureFileSync(logPath.value);
    return outputFile(logPath.value, '')
        .then(() => {
            message.success('操作成功');
            resetLog();
        })
        .catch((err: Error) => {
            message.error(err.message || '操作失败');
        });
}

function handleOpenFileLocation() {
    return shell.showItemInFolder(logPath.value);
}

function dealLogMessage(line: string): string {
    line = line
        .replace(/warning/g, '<span class="warning"> warning </span>')
        .replace(/ error /g, '<span class="error"> error </span>')
        .replace(/ debug /g, '<span class="debug"> debug </span>')
        .replace(/ trace /g, '<span class="trace"> trace </span>');

    if (line.indexOf('critical') !== -1) {
        line = `<span class="critical">${line}</span>`;
    }
    return line;
}

function cleanSearchRelatedState() {
    totalResultCount.value = 0;
    currentResultPointerIndex.value = 0;
    searchResultList = [];
    logList.list.forEach((item: KfLogData) => {
        item.messageForSearch = '';
    });
    clipboard.clear();
}

function resetLog() {
    cleanSearchRelatedState();
    logList.list = [];
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
                            v-model:checked="scrollToBottomChecked"
                            @change="handleSwitchScrollToBottomChecked"
                        >
                            滚动到底部
                        </a-checkbox>
                    </KfDashboardItem>
                    <KfDashboardItem>
                        <div class="search-in-table__warp">
                            <a-input-search
                                ref="inputSearchRef"
                                class="search-int-table__item"
                                v-model:value="searchKeyword"
                                placeholder="关键字"
                                style="width: 120px"
                            />
                            <div
                                class="current-to-total search-int-table__item"
                            >
                                {{ currentResultPointerIndex }} /
                                {{ totalResultCount }}
                            </div>
                            <div
                                class="find-up-down search-int-table__item kf-actions__warp"
                            >
                                <up-outlined
                                    style="font-size: 14px; margin-left: 0px"
                                    @click="handleToUpSearchResult"
                                />
                                <down-outlined
                                    style="font-size: 14px; margin-left: 8px"
                                    @click="handleToDownSearchResult"
                                />
                            </div>
                        </div>
                    </KfDashboardItem>
                    <KfDashboardItem>
                        <a-button size="small" @click="handleOpenFileLocation">
                            文件夹
                        </a-button>
                    </KfDashboardItem>
                    <KfDashboardItem>
                        <a-button
                            size="small"
                            type="primary"
                            @click="handleRemoveLog"
                        >
                            清空
                        </a-button>
                    </KfDashboardItem>
                </template>

                <DynamicScroller
                    id="kf-log-table"
                    v-if="!initLogLoading"
                    ref="scrollerTableRef"
                    class="kf-table"
                    :items="logList.list"
                    :min-item-size="36"
                    :simple-array="true"
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
                                :id="`log-item-${item.id}`"
                                v-html="item.messageForSearch || item.message"
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

        .kf-dashboard__header {
            .kf-dashboard-item__warp {
                .search-in-table__warp {
                    display: flex;
                    justify-content: flex-start;
                    align-items: center;
                    font-size: 12px;
                    margin-right: 4px;

                    .search-int-table__item {
                        margin: 0 4px;
                    }
                }
            }
        }

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

        .search-keyword {
            background: fade(@white, 60%);
            color: #000;
            font-weight: normal;

            &.current-search-pointer {
                background: @primary-color;
                color: #fff;
            }
        }
    }
}
</style>
