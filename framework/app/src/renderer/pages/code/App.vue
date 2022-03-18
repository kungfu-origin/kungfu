<script setup lang="ts">
    import { nextTick, onBeforeMount, onMounted, reactive, ref, toRef } from 'vue';
    import { getLogProcessId, useLogInit } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/logUtils';
    import { removeLoadingMask, setHtmlTitle } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
    import Editor from './components/Editor.vue';
    import FileTree from './components/FileTree.vue';
    import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';

    import { setTimeout } from 'timers';

    // const strategy = reactive({})


    // const handleUpdateStrategy = (() => {
    //     if (!strategy.strategy_id) return;
    //     updateStrategy(strategy.strategy_id);
    // })
    const ProcessId = getLogProcessId();

    const {
        logList,
        logPath,
        scrollToBottomChecked,
        scrollerTableRef,
        scrollToBottom,
        startTailLog,
        clearLogState,
    } = useLogInit(ProcessId);

    setHtmlTitle(`功夫交易系统 - ${ProcessId}.log`);

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

    onMounted(() => {
        removeLoadingMask();
    })
</script>


<template>
    <div class="code-content">
        <FileTree class="file-tree"></FileTree>
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