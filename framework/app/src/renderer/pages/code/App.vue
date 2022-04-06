<script setup lang="ts">
    import { nextTick, onMounted, reactive } from 'vue';
    import { getLogProcessId } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/logUtils';
    import { removeLoadingMask, setHtmlTitle } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
    import Editor from './components/MonacoEditor.vue';
    import FileTree from './components/FileTree.vue';
    import { getStrategyById } from '@kungfu-trader/kungfu-js-api/kungfu/strategy';
    import { ClusterOutlined } from '@ant-design/icons-vue';
    import { useCodeStore } from './store/codeStore';
    // import { ipcEmitDataByName } from '../../../renderer/ipcMsg/emitter'
    import { message } from 'ant-design-vue';
    const store = useCodeStore();
    const ProcessId = getLogProcessId();
    // ipcEmitDataByName('strategyList').then(data => {
    //     console.log(data);
        
    // })
    store.setStrategyList();

    

    
    setHtmlTitle(`功夫交易系统 - ${ProcessId}.log`);
    const strategy = reactive<Code.Strategy>({
        strategy_id: '',
        strategy_path: '',
        add_time: 0
    })
    
    // 处理JSON格式strangeList
    function handleStrategyJsonList (strategyList): void {

        let value: Code.Strategy = JSON.parse(strategyList[0].value)
        
        strategy.strategy_id = value.strategy_id
        strategy.strategy_path = value.strategy_path
        strategy.add_time = value.add_time
    }

    // 处理Object格式strageList
    function handleStrategyList (strategyList): void {

        let value: Code.Strategy = strategyList[0]
        
        strategy.strategy_id = value.strategy_id
        strategy.strategy_path = value.strategy_path
        strategy.add_time = value.add_time
    }

    function handleUpdateStrategy(strategyPath) {
        if (!strategy.strategy_id) {
            message.error('策略id不存在!')
            return
        };
        updateStrategy(strategy.strategy_id, strategyPath);
    }

    async function updateStrategy(strategyId: string, strategyPath: string) {
        const strategyList: Array<Code.Strategy> = await getStrategyById(strategyId)
        handleStrategyList(strategyList)
    }
   
    
    let shouldClose: boolean = false

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
        removeLoadingMask();
        nextTick().then(() => {
            handleStrategyJsonList(store.strategyList);
        })

        store.getKungfuConfig()
        bindCloseWindowEvent()
    })

</script>

<template>
    <div>
        <div class="code-content">
            <FileTree
                :strategy="strategy"
                @updateStrategy="handleUpdateStrategy"
                class="file-tree"
            ></FileTree>
            <Editor class="editor" ref="code-editor"></Editor>
        </div>
        <ClusterOutlined style="font-size: 14px; padding-right: 4px" />
        <span class="title">缩进</span>
    </div>
</template>

<style lang="less" scoped>
#app {
    width: 100%;
    height: 100%;
    font-family: Consolas, Monaco, Lucida Console, Liberation Mono,
    DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale; 
    text-align: center;
    .code-content {
        width: auto;
        display: flex;
        height: 100%;
        .file-tree {
            width: 300px;
        }
        .editor {
            flex: 1;
        }
    }
}
</style>