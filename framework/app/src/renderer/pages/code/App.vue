<script setup lang="ts">
    import { nextTick, onMounted, reactive } from 'vue';
    import { getLogProcessId } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/logUtils';
    import { removeLoadingMask, setHtmlTitle } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
    import Editor from './components/Editor.vue';
    import FileTree from './components/FileTree.vue';

    import { useCodeStore } from './store/codeStore'

    const store = useCodeStore();
    const ProcessId = getLogProcessId();
    store.setStrategyList();

    

    
    setHtmlTitle(`功夫交易系统 - ${ProcessId}.log`);

    // const boardSize = ref<{ width: number; height: number }>({
    //     width: 0,
    //     height: 0,
    // });
    // const handleChangeBoardSize = ({
    //     width,
    //     height,
    // }: {
    //     width: number;
    //     height: number;
    // }) => {
    //     boardSize.value.width = width;
    //     boardSize.value.height = height;
    // };


    const strategy = reactive<Strategy>({
        strategy_id: '',
        strategy_path: '',
        add_time: 0
    })
        
    function getCurrentStrategy (strategyList): void {

        let value: Strategy = JSON.parse(strategyList[0].value)
        
        strategy.strategy_id = value.strategy_id
        strategy.strategy_path = value.strategy_path
        strategy.add_time = value.add_time
    }
    onMounted(() => {
        removeLoadingMask();
        nextTick().then(() => {
            getCurrentStrategy(store.strategyList);
        })
        store.getKungfuConfig()
        
    })

</script>

<template>
    <div class="code-content">
        <FileTree
            :strategy="strategy"
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