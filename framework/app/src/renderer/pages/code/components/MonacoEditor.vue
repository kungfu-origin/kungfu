<template>
  <div class="code-editor">
    <div id="editor-content" v-if="activeFile !== null && !activeFile.isDir"></div>
    <i class="iconfont tr-logo" v-else></i>
  </div>
</template>
<script setup lang="ts">
import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import * as monaco from 'monaco-editor';
import { storeToRefs } from 'pinia';
import { computed, nextTick, ref, watch } from 'vue';
import languageJSON from '../config/iconFileConfig.json';
import themeData from '../config/Monocai.json';
import {
  keywordsList,
  kungfuFunctions,
  kungfuProperties,
  kungfuKeywords,
  pythonKeywords,
} from '../hint/monaco.python.hint';
import { useCodeStore } from '../store/codeStore';
import { getFileContent } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import path from 'path';
import fse from 'fs-extra';

monaco.editor.defineTheme(
  'monokai',
  themeData as monaco.editor.IStandaloneThemeData,
);
monaco.editor.setTheme('monokai');

monaco.languages.registerCompletionItemProvider('python', {
  provideCompletionItems: pythonProvideCompletionItems,
});

// currentFile
const { currentFile, fileTree, globallSetting } = storeToRefs(useCodeStore());
const code = computed(() => globallSetting['code']);

const handleEditor: {
  value: monaco.editor.IStandaloneCodeEditor | null;
} = {
  value: null,
};
const activeFile = ref<Code.FileData | null>(null);

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
    activeFile.value = null;
    handleEditor.value = null;
  }
});

// 监听当前文件状态
watch(currentFile, async (newFile: Code.FileData) => {
  const filePath: string = newFile.filePath || '';

  if (currentFile.value.isDir) return;
  
  clearState();
  activeFile.value = newFile as Code.FileData;
  const codeText: string = await getFileContent(filePath);
  await nextTick();
  if (activeFile.value) {
    handleEditor.value = buildEditor(handleEditor.value, activeFile.value, codeText);
    await nextTick();
    // updateSpaceTab(code.value);
    bindBlur(handleEditor.value, activeFile.value);
  }
});

function bindBlur(editor, curFile) {
  editor !== null &&
    editor.onDidBlurEditorText(() => {
      curWriteFile(editor, curFile);
    });
}

function curWriteFile(editor, curFile) {
  const value = editor.getValue();
  let curPath: string = path.normalize(curFile.filePath);
  fse.outputFile(curPath, value);
}

// 创建代码编辑器
function createEditor(
  file: Code.FileData,
  codeText: string,
): monaco.editor.IStandaloneCodeEditor | null {
  const $editorContent = document.getElementById('editor-content');
  if (!$editorContent) {
    return null;
  }

  $editorContent.innerHTML = '';
  const fileLanguage: string = getFileLanguage(file);
  const editor: monaco.editor.IStandaloneCodeEditor = monaco.editor.create(
    $editorContent,
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

function getFileLanguage(file: Code.FileData | null): string {
  if (file) {
    return file.ext ? languageJSON[file.ext] : 'plaintext';
  }

  return 'plaintext';
}

// 更新代码编辑器
function updateEditor(
  editor: monaco.editor.IStandaloneCodeEditor,
  file: Code.FileData,
  codeText: string,
): monaco.editor.IStandaloneCodeEditor {
  editor.getModel()?.setValue(codeText);
  const fileLanguage = file.ext
    ? languageJSON[file.ext] || 'plaintext'
    : 'plaintext';
  const model = editor.getModel();
  if (model) {
    monaco.editor.setModelLanguage(model, fileLanguage);
  }
  return editor;
}

// 构建代码编辑器
function buildEditor(
  editor: monaco.editor.IStandaloneCodeEditor | null,
  file: Code.FileData,
  codeText: string,
) {
  if (!editor) {
    return createEditor(file, codeText);
  } else {
    return updateEditor(
      editor as monaco.editor.IStandaloneCodeEditor,
      file,
      codeText,
    );
  }
}

// 更新缩进设置
function updateSpaceTab(spaceTabSetting: Code.ICodeSetting) {
  const type: string = spaceTabSetting
    ? spaceTabSetting.tabSpaceType || 'spaces'
    : 'spaces';

  if (handleEditor.value) {
    const model = handleEditor.value.getModel();

    if (!model) return;

    if (type.toLowerCase() === 'spaces') {
      model.updateOptions({
        insertSpaces: true,
        indentSize: +spaceTabSetting.tabSpaceSize,
        tabSize: +spaceTabSetting.tabSpaceSize,
      });
    } else if (type.toLowerCase() === 'tabs') {
      model.updateOptions({
        insertSpaces: false,
        indentSize: +spaceTabSetting.tabSpaceSize,
        tabSize: +spaceTabSetting.tabSpaceSize,
      });
    }
  }
}

// 使用完成销毁实例
function clearState(): void {
  handleEditor.value && handleEditor.value.dispose();
  handleEditor.value = null;
  activeFile.value = null;
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
#editor-content {
    height: 100%;
    .code-editor {
        height: 100%;
    }
}

</style>
