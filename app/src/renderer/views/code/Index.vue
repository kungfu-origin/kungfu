<template>
<MainContent :ifSideBar="false">
    <div class="code-content">
        <FileTree :strategy="strategy" @updateStrategy="handleUpdateStrategy"></FileTree>
        <Editor ref='code-editor'></Editor>
    </div>
</MainContent>
</template>
<script>

import Editor from './components/MonacoEditor';
import FileTree from './components/FileTree';
import MainContent from '@/components/Layout/MainContentForCode'

import { ipcEmitDataByName } from '@/ipcMsg/emitter';
import { setTimeout } from 'timers';

export default {
    name: 'kungfu-code-editor',
    
    data(){
        return {
            strategy: {},
        }
    },

    components: {
        FileTree, Editor,
        MainContent
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
            const { data } = await ipcEmitDataByName('strategyById', { strategyId })
            this.strategy = data[0]
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

