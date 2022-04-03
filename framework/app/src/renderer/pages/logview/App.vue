<script setup lang="ts">
import { nextTick, onMounted, ref } from 'vue';
import {
  UpOutlined,
  DownOutlined,
  ReloadOutlined,
} from '@ant-design/icons-vue';

import {
  removeLoadingMask,
  setHtmlTitle,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import { ensureFileSync, outputFile } from 'fs-extra';
import { message } from 'ant-design-vue';
import { shell } from '@electron/remote';
import { clipboard } from 'electron';
import { platform } from 'os';
import {
  getLogPath,
  useLogInit,
  useLogSearch,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/logUtils';

const LOG_PATH = getLogPath();
setHtmlTitle(`功夫交易系统 - ${LOG_PATH}`);

const boardSize = ref<{ width: number; height: number }>({
  width: 0,
  height: 0,
});

const handleChangeBoardSize = ({
  width,
  height,
}: {
  width: number;
  height: number;
}) => {
  boardSize.value.width = width;
  boardSize.value.height = height;
};

const {
  logList,
  scrollToBottomChecked,
  scrollerTableRef,
  scrollToBottom,
  startTailLog,
  clearLogState,
} = useLogInit(LOG_PATH);

const {
  inputSearchRef,
  searchKeyword,
  currentResultPointerIndex,
  totalResultCount,
  clearLogSearchState,
  handleToDownSearchResult,
  handleToUpSearchResult,
} = useLogSearch(logList, scrollerTableRef, boardSize);

onMounted(() => {
  removeLoadingMask();
  resetLog();
});

document.addEventListener('keydown', (e) => {
  if (process.env.RENDERER_TYPE !== 'logview') return;

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

function handleRemoveLog(): Promise<void> {
  ensureFileSync(LOG_PATH);
  return outputFile(LOG_PATH, '')
    .then(() => {
      message.success('操作成功');
      resetLog();
    })
    .catch((err: Error) => {
      message.error(err.message || '操作失败');
    });
}

function handleOpenFileLocation() {
  return shell.showItemInFolder(LOG_PATH);
}

function resetLog() {
  clearLogState();
  clearLogSearchState();
  console.log(555);
  startTailLog();
}
</script>
<template>
  <a-layout>
    <div class="kf-log-view__warp">
      <KfDashboard @boardSizeChange="handleChangeBoardSize">
        <template v-slot:header>
          <KfDashboardItem>
            <a-checkbox
              size="small"
              v-model:checked="scrollToBottomChecked"
              @change="scrollToBottom"
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
              <div class="current-to-total search-int-table__item">
                {{ currentResultPointerIndex }} /
                {{ totalResultCount }}
              </div>
              <div class="find-up-down search-int-table__item kf-actions__warp">
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
            <a-button size="small">
              <template #icon>
                <ReloadOutlined class="kf-hover" style="font-size: 14px" />
              </template>
            </a-button>
          </KfDashboardItem>
          <KfDashboardItem>
            <a-button size="small" @click="handleOpenFileLocation">
              文件夹
            </a-button>
          </KfDashboardItem>
          <KfDashboardItem>
            <a-button size="small" type="primary" @click="handleRemoveLog">
              清空
            </a-button>
          </KfDashboardItem>
        </template>

        <DynamicScroller
          id="kf-log-table"
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
              item: KungfuApi.KfLogData,
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
                :active="active"
                :id="`kf-log-item-${item.id}`"
                class="kf-log-line"
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
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/base.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/public.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/coverAnt.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/variables.less';

#app {
  width: 100%;
  height: 100%;
  font-family: Consolas, Monaco, Lucida Console, Liberation Mono,
    DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace;
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
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .debug {
      color: @blue-6;
      font-weight: bold;
    }

    .info {
      color: @green2-base;
      font-weight: bold;
    }

    .warning {
      color: @orange-6;
      font-weight: bold;
    }

    .trace {
      color: @cyan-6;
      font-weight: bold;
    }

    .critical {
      color: lighten(@red2-base, 10%);
      font-weight: bold;
    }

    .search-keyword {
      background: fade(@white, 70%);
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
