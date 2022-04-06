import * as monaco from 'monaco-editor';
import fse from 'fs-extra';
import path from 'path';

declare const global: NodeJS.Global;

export const kungfuFunctions = fse
  .readFileSync(path.join(global.__resources, 'keywords', 'kungfuFunctions'))
  .toString()
  .split('\n')
  .map((k: string) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Function,
    documentation: '',
    insertText: k,
  }));

export const kungfuProperties = fse
  .readFileSync(path.join(global.__resources, 'keywords', 'kungfuProperties'))
  .toString()
  .split('\n')
  .map((k: string) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Property,
    documentation: '',
    insertText: k,
  }));

export const kungfuKeywords = fse
  .readFileSync(path.join(global.__resources, 'keywords', 'kungfuKeywords'))
  .toString()
  .split('\n')
  .map((k: string) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Keyword,
    documentation: '',
    insertText: k,
  }));

export const pythonKeywords = fse
  .readFileSync(path.join(global.__resources, 'keywords', 'pythonKeywords'))
  .toString()
  .split('\n')
  .map((k: string) => ({
    label: k,
    kind: monaco.languages.CompletionItemKind.Keyword,
    documentation: '',
    insertText: k,
  }));

export const keywordsList: string[] = [
  ...kungfuKeywords,
  ...pythonKeywords,
  ...kungfuProperties,
  ...kungfuFunctions,
].map((word) => word.label);
