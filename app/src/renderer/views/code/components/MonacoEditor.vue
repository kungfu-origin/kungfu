<template>
  <div class="code-editor">
    <div id="editor-content" v-if="file !== null && !file.isDir"></div>
    <i class="iconfont tr-logo" v-else></i>
  </div>
</template>
<script>
import * as monaco from "monaco-editor/esm/vs/editor/editor.main.js";

import { keywordsList, kungfuFunctions, kungfuProperties, kungfuKeywords, pythonKeywords } from "../hint/monaco.python.hint";
import { mapState } from "vuex";
import * as CODE_UTILS from "__gUtils/fileUtils";
import { delayMiliSeconds } from '__gUtils/busiUtils';

import languageJSON from '../config/iconFileConfig.json';
import themeData from "../config/Monokai.json";

monaco.editor.defineTheme("monokai", themeData);
monaco.editor.setTheme("monokai");

monaco.languages.registerCompletionItemProvider('python', {
    provideCompletionItems: pythonProvideCompletionItems
})


export default {
    data() {
        return {
            file: null,
            editor: null
        };
    },
    mounted() {
        
    },

    computed: {
        ...mapState({
            currentFile: state => state.currentFile,
            fileTree: state => state.fileTree,
            code: state => state.kfConfig.code || {}
        })
    },

    watch: {
        async currentFile(newFile) {
            const filePath = newFile.filePath;
            
            if (this.currentFile.isDir) return; 

            this.clearState(); // 清空状态
            this.file = newFile;
            const codeText = await CODE_UTILS.getCodeText(filePath);
            
            await this.$nextTick();
            this.editor = this.buildEditor(this.editor, this.file, codeText);

            await this.$nextTick();
            this.updateSpaceTab(this.code)
            this.bindBlur(this.editor, this.file)
        },

        fileTree(newTree, oldTree) {
            const newRootPath = Object.values(newTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            const oldRootPath = Object.values(oldTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            if (newRootPath != oldRootPath) {
                this.file = null;
                this.editor = null;
            }
        },

        code(spaceTabSetting) {
            this.updateSpaceTab(spaceTabSetting || {})
        }
    },

    methods: {
        //创建或更新editor
        buildEditor(editor, file, codeText){
            if(!editor){
                return this.createEditor(file, codeText)
            }else{
                return this.updateEditor(this.editor, file, codeText)
            }
        },

        //如果不存在editor，新建
        createEditor(file, codeText) {
            if (document.getElementById("editor-content")) {
                    document.getElementById("editor-content").innerHTML = "";
                    const fileLanguage = languageJSON[file.ext] || 'plaintext';
                    const editor = monaco.editor.create(
                        document.getElementById("editor-content"),
                        {
                            value: codeText || "",
                            language: fileLanguage,

                            autoIndent: true,
                            formatOnPaste: true,
                            formatOnType: true,

                            fontSize: "14",                            
                            automaticLayout: true,
                        }
                    );     
                return editor;
            }
            return null;
        },

        //如果已经存在editor，则更新
        updateEditor(editor, file, codeText){
            editor.updateOptions({
                value: codeText, 
            })
            const fileLanguage = languageJSON[file.ext] || 'plaintext';
            monaco.editor.setModelLanguage(
                editor.getModel(),
                fileLanguage
            )
            return editor;
        },

        bindBlur(editor, file){
            editor !== null && editor.onDidBlurEditorText(e => {
                this.writeFile(editor, file)
            });
        },

        writeFile(editor, file){
            const value = editor.getValue();
            CODE_UTILS.writeFile(file.filePath, value);
        },

        clearState(){
            this.editor && this.editor.dispose();
            this.editor = null;
            this.file = null;
        },

        updateSpaceTab(spaceTabSetting) {
            const type = spaceTabSetting.tabSpaceType || 'spaces';

            if(!this.editor) return;
            
            const model = this.editor.getModel()
            if(type.toLowerCase() === 'spaces') {
                model.updateOptions({
                    insertSpaces: true,
                    indentSize: spaceTabSetting.tabSpaceSize,
                    tabSize: spaceTabSetting.tabSpaceSize,
                })
            } else if (type.toLowerCase() === 'tabs') {
                model.updateOptions({
                    insertSpaces: false,
                    indentSize: spaceTabSetting.tabSpaceSize,
                    tabSize: spaceTabSetting.tabSpaceSize,
                })
            }
        }
    }
};

function pythonProvideCompletionItems (model, position, context, token) {
    const lastChars = model.getValueInRange({
        startLineNumber: position.lineNumber,
        startColumn: 0,
        endLineNumber: position.lineNumber,
        endColumn: position.column
    })

    const charSplitSpace = lastChars.split(' ');
    const ifFunction = (charSplitSpace.length >= 2) && charSplitSpace[charSplitSpace.length - 2]
    if(ifFunction === 'def') return { suggestions: kungfuFunctions }

    const charSplitPoint = lastChars.split('.');
    const ifProperty = (charSplitPoint.length > 1) && charSplitPoint[charSplitPoint.length - 1].indexOf(' ') === -1;
    if(ifProperty) return { suggestions: kungfuProperties }


    const allChars = model.getValueInRange({
        startLineNumber: 0,
        startColumn: 0,
        endLineNumber: position.lineNumber,
        endColumn: position.column
    })

    const allCharList = allChars
        .replace(/\./g, ' ')
        .replace(/@/g,  ' ')
        .replace(/"/g,  ' ')
        .replace(/'/g,  ' ')
        .replace(/:/g,  ' ')
        .replace(/\//g, ' ')
        .replace(/,/g,  ' ')
        .split('\n')
        .join(' ')
        .split(' ')
        .filter(char => (char !== '') && isNaN(char))
        .removeRepeat()
        .filter(char => keywordsList.indexOf(char) == -1)
        .map(char => {
            return {
                label: char,
                kind: monaco.languages.CompletionItemKind.Text,
                documentation: "",
                insertText: char,
            }
        })
    return { suggestions: [...pythonKeywords, ...kungfuKeywords, ...allCharList] }
}

</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.code-editor {
    flex: 1;
    background: $bg_card;

    #editor-content {
        overflow: hidden;
        width: 100%;
        height: 100%;

        /* Make horizontal scrollbar, decorations overview ruler and vertical scrollbar arrows opaque */
        .vs-dark .monaco-scrollable-element > .scrollbar > .slider {
        background: $bg_light;
        }
        .monaco-editor .lines-content .cigra {
        box-shadow: 1px 0 0 0 $input_bg inset;
        }
    }

    .tr-logo {
        color: $bg;
        font-size: 160px;
        display: block;
        margin: auto;
        text-align: center;
        position: relative;
        top: 45%;
        transform: translateY(-50%);
    }
}
</style>



