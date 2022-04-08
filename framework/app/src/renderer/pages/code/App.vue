<script setup lang="ts">
import { nextTick, onMounted, reactive } from 'vue';
import { getProcessId } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/codeUtils';
import {
  removeLoadingMask,
  setHtmlTitle,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import Editor from './components/MonacoEditor.vue';
import FileTree from './components/FileTree.vue';
import { useCodeStore } from './store/codeStore';
import { ipcEmitDataByName } from '../../../renderer/ipcMsg/emitter';
import { message } from 'ant-design-vue';
import MainContentVue from './components/MainContent.vue';

const store = useCodeStore();
const ProcessId: string = getProcessId();

setHtmlTitle(`功夫交易系统 - ${ProcessId}.log`);
const strategy = reactive<Code.Strategy>({
  strategy_id: '',
  strategy_path: '',
  add_time: 0,
});
const strategyName = ProcessId.split('_')[1]
const curnStrategyIndex: {
    value: number
} = {
    value: 0
}
// 处理JSON格式strangeList
function handleStrategyJsonList(strategyList): void {
  getCurrentStrategy(strategyList)
  let value: Code.Strategy = JSON.parse(strategyList[curnStrategyIndex.value].value);
  strategy.strategy_id = value.strategy_id;
  strategy.strategy_path = value.strategy_path;
  strategy.add_time = value.add_time;
}

function getCurrentStrategy (strategyList) {
    strategyList.forEach((item, index) => {
        if (item.name === strategyName) {
            curnStrategyIndex.value = index
        }
    })
}

// 处理Object格式strageList
function handleStrategyList(strategyList): void {
  let value: Code.Strategy = strategyList[0];

  strategy.strategy_id = value.strategy_id;
  strategy.strategy_path = value.strategy_path;
  strategy.add_time = value.add_time;
}

function handleUpdateStrategy(strategyPath) {
  if (!strategy.strategy_id) {
    message.error('策略id不存在!');
    return;
  }
  updateStrategy(strategy.strategy_id, strategyPath);
}

async function updateStrategy(strategyId: string, strategyPath: string) {
  await getStrategyById(strategyId);
}

let shouldClose: boolean = false;

async function getStrategyById(strategyId: string) {
    const { data } = await ipcEmitDataByName('strategyById', { strategyId }) as Record<string, Array<Code.Strategy>>;
    handleStrategyList(data);
}

function bindCloseWindowEvent() {
  shouldClose = false;

  window.onbeforeunload = (e) => {
    e.preventDefault(e);
    if (shouldClose) return undefined;
    const $textareaList = document.querySelectorAll('textarea');
    $textareaList.forEach(($textarea) => {
      $textarea && $textarea.blur();
    });
    shouldClose = true;
    setTimeout(() => {
      window.close();
    }, 100);
    return false;
  };
}
onMounted(() => {
    ipcEmitDataByName('strategyList').then(({data}) => {
        store.setStrategyList(data)
        nextTick().then(() => {
            handleStrategyJsonList(store.strategyList);
        });
    });
    removeLoadingMask();

    store.getKungfuConfig();
    bindCloseWindowEvent();
});
</script>

<template>
  <div class="code-editor-background">
    <MainContentVue>
        <div class="code-content">
        <FileTree
            :strategy="strategy"
            @updateStrategy="handleUpdateStrategy"
        ></FileTree>
        <Editor class="editor" ref="code-editor"></Editor>
        </div>
    </MainContentVue>
  </div>
</template>

<style lang="less">
.code-editor-background {
    width: 100%;
    .code-content {
        width: auto;
        display: flex;
        height: calc(100vh - 32px);
        .editor {
            text-align: left;
            flex: 1;
        }
    }
}
</style>
