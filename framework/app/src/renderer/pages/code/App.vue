<script setup lang="ts">
    import { nextTick, onMounted, reactive } from 'vue';
    import { getLogProcessId } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/logUtils';
    import { removeLoadingMask, setHtmlTitle } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
    // import Editor from './components/Editor.vue';
    import Editor from './components/MonacoEditor.vue';
    import FileTree from './components/FileTree.vue';
    import { ipcEmitDataByName } from './emitter';
    
    import { useCodeStore } from './store/codeStore'

    const store = useCodeStore();
    const ProcessId = getLogProcessId();
    store.setStrategyList();

    

    
    setHtmlTitle(`功夫交易系统 - ${ProcessId}.log`);

    const strategy = reactive<Code.Strategy>({
        strategy_id: '',
        strategy_path: '',
        add_time: 0
    })
        
    function getCurrentStrategy (strategyList, jsonDone: boolean = false): void {

        let value: Code.Strategy = jsonDone ? JSON.parse(strategyList[0].value) : strategyList[0]
        
        strategy.strategy_id = value.strategy_id
        strategy.strategy_path = value.strategy_path
        strategy.add_time = value.add_time
    }

    function handleUpdateStrategy(strategyPath) {
        
      if (!strategy.strategy_id) return;
      updateStrategy(strategy.strategy_id, strategy.strategy_path);
    }

    async function updateStrategy(strategyId: string, strategyPath: string) {
        
        // const { data } = await ipcEmitDataByName('strategyById', { strategyId, strategyPath });
        // getCurrentStrategy(data)
        // console.log(data);
        
        // console.log(strategy);

        
    }


    onMounted(() => {
        removeLoadingMask();
        nextTick().then(() => {
            getCurrentStrategy(store.strategyList, true);
        })

        store.getKungfuConfig()
        
    })

</script>

<template>
    <div class="code-content">
        <FileTree
            :strategy="strategy"
            @updateStrategy="handleUpdateStrategy"
            class="file-tree"
        ></FileTree>
        <Editor class="editor" ref="code-editor"></Editor>
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
            min-width: 300px;
            flex: 1;
        }
        .editor {
            flex: 2;
        }
    }
}
</style>