import * as monaco from 'monaco-editor/esm/vs/editor/editor.api';

import fse from 'fs-extra';
import path from 'path';

export const kungfuFunctions = fse
  .readFileSync(path.join(__resources, 'keywords', 'kungfuFunctions'))
  .toString()
  .split('\n')
  .map((k) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Function,
    documentation: '',
    insertText: k,
  }));

export const kungfuProperties = fse
  .readFileSync(path.join(__resources, 'keywords', 'kungfuProperties'))
  .toString()
  .split('\n')
  .map((k) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Property,
    documentation: '',
    insertText: k,
  }));

export const kungfuKeywords = fse
  .readFileSync(path.join(__resources, 'keywords', 'kungfuKeywords'))
  .toString()
  .split('\n')
  .map((k) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Keyword,
    documentation: '',
    insertText: k,
  }));

export const pythonKeywords = fse
  .readFileSync(path.join(__resources, 'keywords', 'pythonKeywords'))
  .toString()
  .split('\n')
  .map((k) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Keyword,
    documentation: '',
    insertText: k,
  }));

export const keywordsList = [
  ...kungfuKeywords,
  ...pythonKeywords,
  ...kungfuProperties,
  ...kungfuFunctions,
].map((word) => word.label);
