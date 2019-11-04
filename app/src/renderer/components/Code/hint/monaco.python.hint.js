import * as monaco from "monaco-editor/esm/vs/editor/editor.api";
import { logger } from '__gUtils/logUtils'

const fs = require('fs');
const path = require('path');

export const kungfuFunctions = fs.readFileSync(path.join(__resources, 'keywords', 'kungfuFunctions'))
    .toString()
    .split('\n')
    .map(k => (
        {
            label: k,
            kind: monaco.languages.CompletionItemKind.Function,
            documentation: "",
            insertText: k,
        }
    ))

export const kungfuProperties = fs.readFileSync(path.join(__resources, 'keywords', 'kungfuProperties'))
    .toString()
    .split('\n')
    .map(k => (
        {
            label: k,
            kind: monaco.languages.CompletionItemKind.Property,
            documentation: "",
            insertText: k
        }
    ))

export const kungfuKeywords = fs.readFileSync(path.join(__resources, 'keywords', 'kungfuKeywords'))
    .toString()
    .split('\n')
    .map(k => (
        {
            label: k,
            kind: monaco.languages.CompletionItemKind.Keyword,
            documentation: "",
            insertText: k,
        }
    ))


export const pythonKeywords = fs.readFileSync(path.join(__resources, 'keywords', 'pythonKeywords'))
    .toString()
    .split('\n')
    .map(k => (
        {
            label: k,
            kind: monaco.languages.CompletionItemKind.Keyword,
            documentation: "",
            insertText: k,
        }
    ))

export const keywordsList = [...kungfuKeywords, ...pythonKeywords, ...kungfuProperties, ...kungfuFunctions].map(word => word.label)
