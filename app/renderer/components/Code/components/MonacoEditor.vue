<template>
  <div class="code-editor">
    <div id="editor-content" v-if="file !== null && !file.isDir"></div>
    <i class="iconfont tr-logo" v-else></i>
  </div>
</template>
<script>
import * as monaco from "monaco-editor";
import "monaco-editor/esm/vs/editor/contrib/find/findController.js";

import { mapState } from "vuex";
import * as CODE_UTILS from "__gUtils/fileUtils.js";

import languageJSON from '../config/iconFileConfig.json';
import themeData from "../config/Monokai.json";
monaco.editor.defineTheme("monokai", themeData);
monaco.editor.setTheme("monokai");

export default {
    data() {
        return {
            file: null,
            editor: null
        };
    },
    mounted() {
        const t = this;
    },

    computed: {
        ...mapState({
            currentFile: state => state.STRATEGY.currentFile,
            fileTree: state => state.STRATEGY.fileTree
        })
    },

    watch: {
        currentFile(newVal) {
            const t = this;
            const filePath = newVal.filePath;
            if (!t.currentFile.isDir) {
                t.clearState(); // 清空状态
                t.file = newVal;
                CODE_UTILS.getCodeText(filePath).then(codeText => {
                    t.$nextTick().then(() => {
                        // t.codeText = res;
                        t.editor = t.createEditor(t.file, codeText);
                        t.editor = t.buildEditor(t.editor, t.file, codeText)
                        setTimeout(() => {
                            t.bindBlur(t.editor, t.file);
                        }, 300)
                    });
                });
            }
        },

        fileTree(newTree, oldTree) {
            const t = this;
            const newRootPath = Object.values(newTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            const oldRootPath = Object.values(oldTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            if (newRootPath != oldRootPath) {
                t.file = null;
                t.editor = null;
            }
        }
    },

    methods: {
        //创建或更新editor
        buildEditor(editor, file, codeText){
            const t = this;
            if(!editor){
                return t.createEditor(file, codeText)
            }else{
                return t.updateEditor(t.editor, file, codeText)
            }
        },

        //如果不存在editor，新建
        createEditor(file, codeText) {
            if (document.getElementById("editor-content")) {
                    document.querySelector("#editor-content").innerHTML = "";
                    const fileLanguage = languageJSON[file.ext] || 'plaintext';
                    const editor = monaco.editor.create(
                        document.getElementById("editor-content"),
                        {
                            value: codeText || "",
                            language: fileLanguage,
                            fontSize: "14",
                            tabSize: '4',
                            autoIndent:true,
                            automaticLayout: true,
                            extraKeys: {
                                "Tab": function(cm){
                                    cm.replaceSelection("   " , "end");
                                }
                            }
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
            const t = this;
            editor !== null && editor.onDidBlurEditorText(e => {
                const value = editor.getValue();
                t.writeFile(editor, file)
            });
        },

        writeFile(editor, file){
            CODE_UTILS.writeFile(file.filePath, value);
        },

        clearState(){
            const t = this;
            t.editor = null;
            t.file = null;
        }
    }
};
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.code-editor {
    width: calc(100% - 300px);
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



