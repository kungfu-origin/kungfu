<template>
  <div class="code-editor">
    <div id="editor-content" v-if="file !== null && !file.isDir"></div>
    <i class="iconfont tr-logo" v-else></i>
  </div>
</template>
<script setup lang="ts">
import * as monaco from 'monaco-editor/esm/vs/editor/editor.main.js';
import { computed, watch, nextTick, toRef, reactive } from 'vue';
import {
  keywordsList,
  kungfuFunctions,
  kungfuProperties,
  kungfuKeywords,
  pythonKeywords,
 } from '../hint/monaco.python.hint';
// import { mapState } from 'vuex';
// kungfu\framework\api\src\utils\fileUtils.ts
import * as CODE_UTILS from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { objectToString } from '@vue/shared';
import { storeToRefs } from 'pinia';
import languageJSON from '../config/iconFileConfig.json';
import { useCodeStore } from '../store/global'
import themeData from '../config/Monocai.json';

monaco.editor.defineTheme('monokai', themeData);






monaco.editor.setTheme('monokai');

monaco.languages.registerCompletionItemProvider('python', {
  provideCompletionItems: pythonProvideCompletionItems,
});




const { currentFile, fileTree, kfConfig } = storeToRefs(useCodeStore())
const code = computed(() => kfConfig['code'])



let editor: monaco.editor;
let file: FileProps;


watch(currentFile, async (newFile: FileProps) => {
    const filePath = newFile.filePath;

    if (currentFile.value.isDir) return;
    clearState();
    file = newFile;
    const codeText = await CODE_UTILS.getCodeText(filePath)
    await nextTick();
    editor = buildEditor(editor, file, codeText);



    await nextTick();

    updateSpaceTab(code.value);
    bindBlur(editor, file);
})
watch(fileTree, (newTree, oldTree) => {
    const newRootPath = Object.values(newTree).map((tree: FileProps) => {
        if (tree.root) return tree.filePath;
    })[0];
    const oldRootPath = Object.values(oldTree).map((tree: FileProps) => {
        if (tree.root) return tree.filePath;
    })[0];
    if (newRootPath != oldRootPath) {
        file = null;
        editor = null;
    }
})
watch(code, spaceTabSetting => {
    updateSpaceTab(spaceTabSetting || {})
})

function bindBlur(editor, file) {
    editor !== null &&
    editor.onDidBlurEditorText((e) => {
        writeFile(editor, file);
    });
}

function writeFile(editor, file) {
    const value = editor.getValue();
    CODE_UTILS.writeFile(file.filePath, value);
}


function buildEditor(editor, file, codeText) {
    if (!editor) {
        return createEditor(file, codeText);
    } else {
        return updateEditor(this.editor, file, codeText);
    }
}

function createEditor(file, codeText) {
    if (document.getElementById('editor-content')) {
        document.getElementById('editor-content').innerHTML = '';
        const fileLanguage = languageJSON[file.ext] || 'plaintext';
        const editor = monaco.editor.create(
            document.getElementById('editor-content'),
            {
                value: codeText || '',
                language: fileLanguage,

                autoIndent: true,
                formatOnPaste: true,
                formatOnType: true,

                fontSize: '14',
                automaticLayout: true,
            },
        );
        return editor;
    }
    return null;
}

function updateEditor(editor, file, codeText) {
    editor.updateOptions({value: codeText});
    const fileLanguage = languageJSON[file.ext] || 'plaintext';
    monaco.editor.setModelLanguage(editor.getModel(), fileLanguage);
    return editor;
}

function clearState() {
    editor && editor.dispose();
    editor = null;
    file = null;
}

function updateSpaceTab(spaceTabSetting) {
    const type = spaceTabSetting.tabSpaceType || 'spaces';

    if (!editor) return;

    const model = editor.getModel();
    if (type.toLowerCase() === 'spaces') {
        model.updateOptions({
            insertSpaces: true,
            indentSize: spaceTabSetting.tabSpaceSize,
            tabSize: spaceTabSetting.tabSpaceSize,
        });
    } else if (type.toLowerCase() === 'tabs') {
        model.updateOptions({
            insertSpaces: false,
            indentSize: spaceTabSetting.tabSpaceSize,
            tabSize: spaceTabSetting.tabSpaceSize,
        });
    }
}





function pythonProvideCompletionItems(model, position, context, token) {
  const lastChars = model.getValueInRange({
    startLineNumber: position.lineNumber,
    startColumn: 0,
    endLineNumber: position.lineNumber,
    endColumn: position.column,
  });

  const charSplitSpace = lastChars.split(' ');
  const ifFunction =
    charSplitSpace.length >= 2 && charSplitSpace[charSplitSpace.length - 2];
  if (ifFunction === 'def') return { suggestions: kungfuFunctions };

  const charSplitPoint = lastChars.split('.');
  const ifProperty =
    charSplitPoint.length > 1 &&
    charSplitPoint[charSplitPoint.length - 1].indexOf(' ') === -1;
  if (ifProperty) return { suggestions: kungfuProperties };

  const allChars = model.getValueInRange({
    startLineNumber: 0,
    startColumn: 0,
    endLineNumber: position.lineNumber,
    endColumn: position.column,
  });

  const allCharList = allChars
    .replace(/\./g, ' ')
    .replace(/@/g, ' ')
    .replace(/"/g, ' ')
    .replace(/'/g, ' ')
    .replace(/:/g, ' ')
    .replace(/\//g, ' ')
    .replace(/,/g, ' ')
    .split('\n')
    .join(' ')
    .split(' ')
    .filter((char) => char !== '' && isNaN(char))
    .removeRepeat()
    .filter((char) => keywordsList.indexOf(char) == -1)
    .map((char) => {
      return {
        label: char,
        kind: monaco.languages.CompletionItemKind.Text,
        documentation: '',
        insertText: char,
      };
    });
  return {
    suggestions: [...pythonKeywords, ...kungfuKeywords, ...allCharList],
  };
}
</script>
<style lang="less"></style>
