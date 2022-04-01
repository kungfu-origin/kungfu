<template>
  <div class="code-editor">
    <div id="editor-content" v-if="file !== null && !file.isDir"></div>
    <i class="iconfont tr-logo" v-else></i>
  </div>
</template>
<script setup lang="ts">
import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import * as monaco from 'monaco-editor/esm/vs/editor/editor.main.js';
import { storeToRefs } from 'pinia';
import { computed, nextTick, reactive, watch } from 'vue';
import languageJSON from '../config/iconFileConfig.json';
import themeData from '../config/Monocai.json';
import {
  keywordsList,
  kungfuFunctions,
  kungfuProperties,
  kungfuKeywords,
  pythonKeywords,
} from '../hint/monaco-python-hint';
import { useCodeStore } from '../store/codeStore';
import { getCodeText } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import path from 'path';
import fse from 'fs-extra';

monaco.editor.defineTheme('monokai', themeData);
monaco.editor.setTheme('monokai');

monaco.languages.registerCompletionItemProvider('python', {
  provideCompletionItems: pythonProvideCompletionItems,
});

// currentFile
const { currentFile, fileTree, kfConfig } = storeToRefs(useCodeStore());
const code = computed(() => kfConfig['code']);

let handleEditor: monaco.editor.IDiffNavigator = reactive({});
let file: Code.FileProps = reactive({});

watch(code, (spaceTabSetting) => {
  updateSpaceTab(spaceTabSetting || {});
});

// 监听文件树变化
watch(fileTree, (newTree, oldTree) => {
  let newRootPath = findTargetFromArray<Code.FileData>(
    Object.values(newTree),
    'root',
    true,
  )!.filePath;

  let oldRootPath = '';
  if (oldTree[0] && oldTree[0].id) {
    oldRootPath = findTargetFromArray<Code.FileData>(
      Object.values(oldTree),
      'root',
      true,
    )!.filePath;
  }
  if (newRootPath !== oldRootPath) {
    file = {};
    handleEditor = null;
  }
});

// 监听当前文件状态
watch(currentFile, async (newFile: Code.FileProps) => {
  const filePath: string = newFile.filePath || '';

  if (currentFile.value.isDir) return;
  clearState();
  file = newFile;
  const codeText: string = await getCodeText(filePath);
  await nextTick();
  handleEditor = buildEditor(handleEditor, file, codeText);
  await nextTick();
  // updateSpaceTab(code.value);
  bindBlur(handleEditor, file);
});

function bindBlur(editor, file) {
  editor !== null &&
    editor.onDidBlurEditorText(() => {
      curWriteFile(editor, file);
    });
}

function curWriteFile(editor, file) {
  const value = editor.getValue();
  let curPath: string = path.normalize(file.filePath);
  fse.outputFile(curPath, value);
}

// 创建代码编辑器
function createEditor(file: Code.FileProps, codeText: string): monaco.editor {
  if (document.getElementById('editor-content')) {
    (document.getElementById('editor-content') as any).innerHTML = '';
    let fileLanguage: string = 'plaintext';
    if (file) {
      fileLanguage = file.ext ? languageJSON[file.ext] : 'plaintext';
    }
    const editor: monaco.editor = monaco.editor.create(
      document.getElementById('editor-content'),
      {
        value: codeText,
        language: fileLanguage,

        autoIndent: 'full',
        formatOnPaste: true,
        formatOnType: true,

        fontSize: 14,
        automaticLayout: true,
      },
    );
    return editor;
  }
  return null;
}

// 更新代码编辑器
function updateEditor(
  editor: monaco.editor,
  file: Code.FileProps,
  codeText: string,
): monaco.editor {
  editor.updateOptions({ value: codeText });
  const fileLanguage = file.ext
    ? languageJSON[file.ext] || 'plaintext'
    : 'plaintext';
  handleEditor.setModelLanguage(editor.getModel(), fileLanguage);
  return editor;
}

// 构建代码编辑器
function buildEditor(
  editor: monaco.editor,
  file: Code.FileProps,
  codeText: string,
) {
  if (!editor) {
    return createEditor(file, codeText);
  } else {
    return updateEditor(handleEditor, file, codeText);
  }
}

// 更新缩进设置
function updateSpaceTab(spaceTabSetting: Code.ICodeSetting) {
  const type: string = spaceTabSetting
    ? spaceTabSetting.tabSpaceType || 'spaces'
    : 'spaces';

  if (handleEditor) {
    const model = handleEditor.getModel();
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
}

// 使用完成销毁实例
function clearState(): void {
  handleEditor && handleEditor.dispose();
  handleEditor = null;
  file = {};
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
<style lang="less">
.code-editor {
  .monaco-editor {
    height: 100vh !important;
  }
}
</style>
<style lang="less" scoped>
.code-editor {
  width: 100%;
  height: 100vh;
}
</style>
