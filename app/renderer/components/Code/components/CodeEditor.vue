<template>
  <div class="code-editor">
    <div id="editor-content" v-if="currentFile !== null && !currentFile.isDir"></div>
    <svg v-else class="icon tr-LOGO-1" :aria-hidden="true">
        <use xlink:href="#tr-LOGO-1"></use>
    </svg>
  </div>
</template>
<script>
import { mapState } from "vuex";
import * as CODE_UTILS from "__gUtils/fileUtils.js";

import CodeMirror from 'codemirror/lib/codemirror'
import 'codemirror/lib/codemirror.css';
import 'codemirror/theme/monokai.css';
//language
import 'codemirror/mode/python/python.js'
import 'codemirror/mode/clike/clike.js'
import 'codemirror/mode/shell/shell.js'
import 'codemirror/mode/sql/sql.js'
import 'codemirror/mode/javascript/javascript.js'

//hint
import 'codemirror/addon/hint/show-hint.js'
import 'codemirror/addon/hint/anyword-hint.js'
import 'codemirror/addon/hint/sql-hint.js'

//addon
import 'codemirror/addon/hint/show-hint.css';
import 'codemirror/addon/search/search.js';
import 'codemirror/addon/search/searchcursor.js';
import 'codemirror/addon/search/jump-to-line.js';
import 'codemirror/addon/dialog/dialog.js';
import 'codemirror/addon/dialog/dialog.css'


import pythonHintFunc from '../hint/taurus.python.hint.js';
import cppHintFunc from '../hint/taurus.cpp.hint.js';

const pythonHint = pythonHintFunc(CodeMirror)
const cppHint = cppHintFunc(CodeMirror)


//mime setting
const mime = require('mime');
const codemirrorMimeTypes = CodeMirror.mimeModes;
const codemirrorHint = CodeMirror.hint


export default {
    data() {
        this.editor = null;
        return {
            // file: null,
            options: {
                theme: 'monokai',
                lineNumbers: true,
                lineWrapping: true,
                indentUnit: 4, // 缩进单位为4
                tabSize: 4,
                indentWithTabs: true,
                styleActiveLine: true, // 当前行背景高亮
                matchBrackets: true, // 括号匹配
                lineWrapping: "wrap", //在长行时文字是换行(wrap)还是滚动(scroll)，默认为滚动(scroll)
                showCursorWhenSelecting: true, //在选择时是否显示光标，默认为false
                cursorHeight: 0.85, //光标高度。默认为1
                lineWrapping: true, //代码折叠
                foldGutter: true, //断点
                gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter", "breakpoints"],
                smartIndent : true, //智能缩进
                autoMatchParens: true,
                extraKeys: {//智能提示
                    Ctrl: 'autocomplete',
                    Tab: function(cm) {
                        var spaces = Array(cm.getOption("indentUnit") + 1).join(" ");
                        cm.replaceSelection(spaces);
                    },
                },
                scrollTimer: null,
                smartIndent: true,
                
            }
        };

    },
    mounted() {
        const t = this;
    },

    computed: {
        ...mapState({
            currentFile: state => state.STRATEGY.currentFile,
            fileTree: state => state.STRATEGY.fileTree
        }),

        //代码提示
        currnetHint(){
            const t = this;
            const mimeType = t.getMimeType(t.currentFile)
            let mimeName = (codemirrorMimeTypes[mimeType] || {}).name || ''
            switch(mimeName){
                case 'clike':
                    return cppHint;
                    break;
                case 'sql':
                    return codemirrorHint.sql
                case '':
                    if(mimeType === 'python'){
                        return pythonHint
                    }else{
                        return codemirrorHint.anyword
                    }
                    break;
                default:
                    return codemirrorHint.anyword
            }
        }
    },

    watch: {
        currentFile(newFile) {
            const t = this;
            if(Object.keys(newFile).length === 1) return
            if(!newFile.isDir) {
                const filePath = newFile.filePath;
                CODE_UTILS.getCodeText(filePath).then(codeText => {
                    t.$nextTick().then(() => {
                        t.editor = t.buildEditor(t.editor, newFile, codeText)
                    });
                }).catch(err => t.$message.error(err));
            }else{
                t.clearState()
            }
        },

        //切换策略路径
        fileTree(newTree, oldTree) {
            const t = this;
            const newRootPath = Object.values(newTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            const oldRootPath = Object.values(oldTree).map(tree => {
                if (tree.root) return tree.filePath;
            })[0];
            if (newRootPath != oldRootPath) {
                t.clearState()
            }
        }
    },

    methods: {
        //创建或更新editor
        buildEditor(editor, file, codeText){
            const t = this;
            if(!editor){
                editor = t.createEditor(file, codeText)
                t.bindEvent(editor)
                return editor
            }else{
                return t.updateEditor(editor, file, codeText)
            }
        },

        //如果不存在editor，新建
        createEditor(file, codeText) {
            const t = this;
            const targetDom = document.getElementById('editor-content')
            if (targetDom) {
                    const editor = CodeMirror(targetDom, {
                        value: codeText,
                        mode: t.getMimeType(file),
                        ...t.options,
                        onBlur: (e) => t.bindBlur(e, file),
                    })
                return editor;
            }
            return null;
        },

        //如果已经存在editor，则更新
        updateEditor(editor, file, codeText){
            const t = this;
            editor.setValue(codeText)
            editor.setOption(
                'mode', 
                t.getMimeType(file)
            )
            return editor;
        },

        bindEvent(cm) {
            const t = this;
            cm.on('blur', () => {
                const value = cm.getValue();
                CODE_UTILS.writeFile(t.currentFile.filePath, value);
            })

            cm.on('keypress', () => {
                cm.showHint({
                    hint: t.currnetHint
                })
            })
        },

        getMimeType(file){
            const t = this;
            const fileName = file.name;
            const ext = file.ext;
            let mimeType = mime.getType(file.name)
            if(mimeType === null){
                if(ext == 'py'){
                    mimeType = 'python'
                }
            }
            return mimeType

        },

        clearState(){
            const t = this;
            t.editor != null && t.editor.getWrapperElement().parentNode.removeChild(t.editor.getWrapperElement());
            t.editor = null;
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
    .CodeMirror, .CodeMirror-gutters{
        background: $bg_card;
    }
  }
  .tr-LOGO-1 {
    color: $bg;
    font-size: 160px;
    display: block;
    margin: auto;
    text-align: center;
    position: relative;
    top: 45%;
    transform: translateY(-50%);
    filter: drop-shadow($bg_light 1px -1px)
  }
}

#tr-LOGO-1 path{
    fill: $bg !important;
}

.CodeMirror.CodeMirror-wrap{
    height: 100%;
}
.CodeMirror-hints{
    max-height: 40em !important;
}
.CodeMirror-gutter-elt{
    left: 0px !important;
}
pre.CodeMirror-line{
    padding-left: 10px; 
}
.CodeMirror.CodeMirror-wrap{
    font-size: 14px;
    line-height: 24px;
    font-family: Menlo, Monaco, 'Courier New', monospace
}
.CodeMirror-linenumber.CodeMirror-gutter-elt{
    color: $input_bg
}
.CodeMirror-code{
   padding-bottom: 300px; 
}

</style>



