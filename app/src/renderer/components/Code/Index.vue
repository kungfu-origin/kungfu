<template>
<main-content :ifSideBar="false">
    <div class="code-content">
        <FileTree :strategy="strategy" @updateStrategy="handleUpdateStrategy"></FileTree>
        <Editor ref='code-editor'></Editor>
    </div>
</main-content>
</template>
<script>

import * as STRATEGY_API from '__io/kungfu/strategy';
import FileTree from './components/FileTree';
import Editor from './components/MonacoEditor';
import { setTimeout } from 'timers';

const { dialog } = require('electron').remote;

export default {
    name: 'kungfu-code-editor',
    
    data(){
        return {
            strategy: {},
        }
    },

    components: {
        FileTree, Editor, 
    },

    mounted(){
        this.setCurrentStrategy();
        this.bindCloseWindowEvent();
    },


    destroyed(){
        window.blur()
    },

    methods: {
        //子组件对strategy做了更改，需要更新策略
        handleUpdateStrategy(){
            if(!this.strategy.strategy_id) return;
            this.updateStrategy(this.strategy.strategy_id)
        },

        setCurrentStrategy () {
            const strategyId = this.$route.params.id;
            this.updateStrategy(strategyId)
        },

        bindCloseWindowEvent () {
            this.shouldClose = false;

            window.onbeforeunload = (e) => {
                e.preventDefault(e)
                if(this.shouldClose) return undefined;
                const $textareaList = document.querySelectorAll('textarea');
                $textareaList.forEach($textarea => {
                    $textarea && $textarea.blur()
                })
                this.shouldClose = true;
                setTimeout(() => {
                    window.close();
                }, 100)
                return false;
            }
        },

        //更新strategy
        async updateStrategy(strategyId){
            const strategies = await STRATEGY_API.getStrategyById(strategyId);
            this.strategy = strategies[0]
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

