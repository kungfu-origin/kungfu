<template>
    <div class="file-tree">
        <div 
        class="open-editor-folder"  
        @click="handleBindStrategyFolder"
        >设置策略入口文件</div>
        <div class="file-tree-content" >
            <div class="strategy-name">
                <span class="name" ><span>{{strategy.strategy_id}}</span>（当前策略）</span>
                <span class="fr folder-oper" title="新建文件夹" v-if="strategyPath" @click="handleAddFolder"><i class="fl iconfont tr-folder1"></i></span>
                <span class="fr folder-oper" title="新建文件" v-if="strategyPath" @click="handleAddFile"><i class="fl iconfont tr-document1"></i></span>
            </div>
            <ul class="file-tree-body" v-if="strategyPath">
                <fileNode 
                v-for="item in fileTree" 
                :fileNode="item" 
                :key="item.fileId"
                v-if="item.root"
                :id="item.id"
                type="folder"
                ></fileNode>
            </ul>
        </div>
    </div>
</template>
<script>
import * as STRATEGY_API from '@/io/db/strategy';
import path from 'path';
import FileNode from './FileNode';
import {mapState} from 'vuex';
import * as CODE_UTILS from "__gUtils/fileUtils.js";

export default {
    props: {
        strategy: {
            type: Object,
            default: {}
        }
    },

    components: {
        FileNode
    },

    data(){
        return {
            strategyPath: '',
            strategyPathName: '',
        }
    },

    computed: {
        ...mapState({
            fileTree: state => state.STRATEGY.fileTree,
            currentFile: state => state.STRATEGY.currentFile,
        })
    },

    mounted(){
        const t = this;
      

    },

    watch: {
        strategy(newStrategy){
            const t = this;
            t.getPath(newStrategy);
            t.initFileTree(newStrategy).then(fileTree => {
                //设置current file
                const entryPath = newStrategy.strategy_path;//入口文件
                const currentFile = Object.values(fileTree || {}).filter(f => f.filePath == entryPath)[0]
                if(currentFile) {
                    t.$store.dispatch('setEntryFile', currentFile)
                    t.$store.dispatch('setCurrentFile', currentFile)
                } 
            });
        },
    },

    methods: {
        //绑定策略
        handleBindStrategyFolder(){
            const t = this;
            const dialog = require('electron').remote.dialog;
            dialog.showOpenDialog({
                properties: ['openFile']
            }, (strategyPath) => {
                if(!strategyPath || !strategyPath[0]) return;
                if(!t.strategy.strategy_id) return;
                t.bindStrategyPath(strategyPath);
                
            })
        },

        //加文件夹
        handleAddFolder(){
            const t =  this;
            const target = t.fileTree[t.currentFile.id];
            if(target.isDir){
                CODE_UTILS.openFolder(t.$store, target, t.fileTree, true).then(() => {
                    t.$store.dispatch('addFileFolderPending', {id: target.id, type: 'folder'})
                });
            }else{
                if(target.parentId !== undefined && !isNaN(target.parentId)){
                    t.$store.dispatch('addFileFolderPending', {id: target.parentId, type: 'folder'})
                }else{
                    throw new Error()
                }
            }
        },

        //加文件
        handleAddFile(){
            const t =  this;
            const target = t.fileTree[t.currentFile.id];
            if(target.isDir){
                CODE_UTILS.openFolder(t.$store, target, t.fileTree, true).then(() => {
                    t.$store.dispatch('addFileFolderPending', {id: target.id})
                });
            }else{
                if(target.parentId !== undefined && !isNaN(target.parentId)){
                    t.$store.dispatch('addFileFolderPending', {id: target.parentId})
                }else{
                    throw new Error()
                }
            }
        },

        //bind data中path 与 sqlite中path
        async bindStrategyPath(strategyPath){
            const t = this;
            await STRATEGY_API.updateStrategyPath(t.strategy.strategy_id, strategyPath[0])
            t.$message.success(`策略${t.strategy.strategy_id}文件路径修改成功！`)
            //每次更新path，需要通知root组件更新stratgy
            t.$emit('updateStrategy')
        },
        

        //通过新打开的窗口获取文件树
        async initFileTree(strategy){
            const t = this;
            const strategyId = strategy.strategy_id;
            const strategyPath = strategy.strategy_path;
            if(!strategyId || !strategyPath) return;
            //根文件夹得信息不像其他通过fs读取，而是直接从props的strategy中去取
            const rootId = window.fileId++;
            const rootFile = CODE_UTILS.buildFileObj({
                id: rootId,
                isDir: true,
                name: t.strategyPathName,
                filePath: t.strategyPath,
                root: true,
                open: true
            })
            //获取第一级文件树
            let ids, fileTree;
            try {
                const fileTreeData = await CODE_UTILS.getTreeByFilePath({strategy: rootFile, fileTree: {}})
                ids = fileTreeData.ids;
                fileTree = fileTreeData.fileTree;
            } catch (err) {
                t.$message.error(err)
            }
            //处理根
            rootFile['children'] = ids;
            fileTree[rootId] = rootFile
            //pending
            fileTree = t.bindFunctionalNode(fileTree)
            t.$store.dispatch('setFileTree', fileTree)
            t.$store.dispatch('setCurrentFile', rootFile)
            return fileTree
        },

        //增加 以pending 为key 的node，为addfile做准备
        bindFunctionalNode(fileTree){
            fileTree['pending'] = {
                parentId: '',
            }
            return fileTree
        }, 

        //从prop内获取path
        getPath(strategy){
            const t = this;
            if(strategy && strategy.strategy_path) {
                //因为绑定策略时是文件，需要提取其父目录
                t.strategyPath = path.dirname(strategy.strategy_path);
                t.strategyPathName = path.basename(t.strategyPath)
            }
        },
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
$fileTreeWidth: 300px;
.file-tree{
    width: $fileTreeWidth;
    padding-top: 8px;
    text-align: center;
    display: flex;
    flex-direction: column;
    background: $bg_dark;

    .open-editor-folder{
        width: 90%;
        height: 36px;
        line-height: 36px;
        color: $font;
        background: $tab_header;
        margin: auto;
        cursor: pointer;
        border-radius: 3px;
    }
    .open-editor-folder:hover{
        background: $bg_light;
        color: #fff;
    }
    .strategy-name{
        font-size: 14px;
        font-weight: bolder;
        margin-top: 8px;
        height: 30px;
        line-height: 30px;
        padding: 0px 8px;
        border-top: none;
        border-bottom: 2px solid $bg_card;
        text-align: left;
        box-sizing: border-box;
        color: $input_bg;
        .name{
            span{
                vertical-align: bottom;
                display: inline-block;
                max-width: 160px;
                text-overflow: ellipsis;
                white-space: nowrap;
                overflow: hidden;
            }
        }
        .folder-oper{
            cursor: pointer;
            display: none;
            padding: 0 6px;
            i{
                font-size: 15px;
            }
        }
    }
    .folder-name{
        height: 30px;
        line-height: 30px;
        color: $font_1;
        text-align: left;
        display: flex;
        flex-direction: row;
        padding-left: 5px;
        .name{
            max-width: 100px;
            font-size: 14px;
            padding-right: 20px;   
            color: $font;
        }
        .title{
            color: $input_bg;
        }
        
    }
    .file-tree-content{
        height: calc(100% - 40px);
    }
    .file-tree-body{
        height: calc(100% - 38px);
        overflow: auto;
        padding-left: 5px;
        li{
            position: relative;
            line-height: 30px;
            font-size: 14px;
            text-align: left;
            color: $font_6;
            cursor: pointer;
            span>i, span>img{
                padding-right: 5px;
                display: inline-block;
                width: 12px;
            }
            span>img.icon{
                width: 16px;
                height: 16px;
                vertical-align: middle;
                padding-left: 24px;
            }
            &.folder>div>span>img.icon{
                padding-left: 5px;
            }
            .file-name-content{
                // overflow:hidden;
                >span{
                    display: inline-block;
                    width: 100%;
                    box-sizing: border-box;
                    >.el-input{
                        width: calc(100% - 51px);
                        &.error{
                            input{
                                border: 1px solid $error;
                            }
                        }
                    }
                    &.is-root{
                        .path{
                            display: inline-block;
                            max-width: 220px;
                            color: $input_bg;
                            padding-left: 10px;
                            vertical-align: bottom;
                        }
                        .name{
                            display: inline-block;
                            max-width: 220px;
                            vertical-align: bottom;
                        }
                    }
                    .error-message{
                        position: absolute;
                        border: 1px solid $error;
                        top: 30px;
                        width: calc(100% - 62px);
                        background: $bg_card;
                        border-radius: 3px;
                        left: 60px;
                        z-index: 999;
                        color: $error !important;
                        padding: 5px 10px;
                        box-sizing: border-box;
                        line-height: 20px;
                        font-size: 12px;
                    }
                }  
                .file-oper{
                    margin-right: 5px;
                    display: none;
                    i{
                        font-size: 12px;
                        padding: 0 2px;
                        cursor: pointer;
                    }
                    i.fa-close{
                        font-size: 14px;
                    }
                    i:hover{
                        color: $font_1;
                    }
                    i.fa-close:hover{
                        color: $error;
                    }
                } 
                .name{
                    max-width: calc(100% - 95px);
                    display: inline-block;
                    vertical-align: bottom;
                     &.import-file{
                        max-width: 90px;
                    }
                }
                &.on-editing{
                    background: none;
                    &::before{
                        content: "";
                        position: fixed;
                        left: 8px;
                        top: 58px;
                        width: $fileTreeWidth;
                        height: calc(100% - 88px);
                        background: rgba(0,0,0,0.5);
                        z-index: 999;
                    }
                    >span{
                        z-index: 1000;
                        position: relative;
                    }
                } 
            }
        }
        li > div:hover{
            color: #fff;
            .file-oper{
                display: block;
            }
        }
        li.active > div{
            background: $tab_header;        
        }

        ul>li>div>span{padding-left: 10px;}
        ul>li>ul>li>div>span{padding-left: 20px;}
        ul>li>ul>li>ul>li>div>span{padding-left: 30px;}
        ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 40px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 50px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 60px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 70px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 80px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 90px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 100px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 110px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 120px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 130px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 140px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 150px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 160px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 170px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 180px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 190px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 200px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 210px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 220px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 230px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 240px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 250px;}
        ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>ul>li>div>span{padding-left: 260px;}
    }
}

.file-tree:hover{
    .folder-oper{
        display: block;
    }
}


</style>

