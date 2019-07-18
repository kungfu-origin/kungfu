<template>
<main-content :ifSideBar="false">
    <div class="code-content">
        <FileTree :strategy="strategy" @updateStrategy="handleUpdateStrategy"></FileTree>
        <Editor ref='code-editor'></Editor>
    </div>
</main-content>
</template>
<script>
import * as STRATEGY_API from '__io/db/strategy';
import FileTree from './components/FileTree';
import Editor from './components/CodeEditor';
import { setTimeout } from 'timers';
const {dialog} = require('electron').remote;
// import DebugBoard from './components/DebugBoard';
export default {
    data(){
        return {
            strategy: {},

        }
    },

    components: {
        FileTree, Editor, 
        // DebugBoard
    },

    mounted(){
        const t = this;
        const locationHash = window.location.hash.toString().split('/');
        const strategyId = locationHash[locationHash.length - 1]
        if(!strategyId) {
            console.error('没有ID！')
            return
        }
        t.updateStrategy(strategyId)

        t.shouldClose = false;

        window.onbeforeunload = (e) => {
            e.preventDefault(e)
            if(t.shouldClose) return undefined;
            const $textareaList = document.querySelectorAll('textarea');
            $textareaList.forEach($textarea => {
                $textarea && $textarea.blur()
            })
            t.shouldClose = true;
            setTimeout(() => {
                window.close();
            }, 100)
            return false;
        }
    },

    destroyed(){
        window.blur()
    },

    methods: {
        //子组件对strategy做了更改，需要更新策略
        handleUpdateStrategy(){
            const t = this;
            if(!t.strategy.strategy_id) return;
            t.updateStrategy(t.strategy.strategy_id)
        },

        //更新strategy
        async updateStrategy(strategyId){
            const t = this;
            const strategys = await STRATEGY_API.getStrategyById(strategyId)
            t.strategy = strategys[0]
        }
    }

    
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
    .code-content{
        height: 100%;
        // background: $bg_card;
        display: flex;
        flex-direction: row;
    }
</style>

