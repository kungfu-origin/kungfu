import '../config/homePathConfig';
import { createI18n } from 'vue-i18n';
import zh_CN from './zh-CN';
import en_US from './en-US';
import path from 'path';
import fse from 'fs-extra';

// 语言库
const messages = {
  'zh-CN': zh_CN,
  'en-US': en_US,
};

export const languageList = [
  { value: 'zh-CN', label: '简体中文' },
  { value: 'en-US', label: 'English' },
];

export const getExtraLanguage = () => {
  const languagePath = path.join(
    globalThis.__publicResources,
    'language',
    'locale.json',
  );
  if (!fse.existsSync(languagePath)) {
    return null;
  } else {
    return fse.readJSONSync(languagePath, { throws: false });
  }
};

export const getMergeCNLanguage = () => {
  const languagePath = path.join(
    globalThis.__publicResources,
    'language',
    'zh-CN-merge.json',
  );
  if (!fse.existsSync(languagePath)) {
    console.log(123);
    return null;
  } else {
    console.log(24234);
    return fse.readJSONSync(languagePath, { throws: false });
  }
};

export const getMergeENLanguage = () => {
  const languagePath = path.join(
    globalThis.__publicResources,
    'language',
    'en-US-merge.json',
  );
  if (!fse.existsSync(languagePath)) {
    return null;
  } else {
    return fse.readJSONSync(languagePath, { throws: false });
  }
};

// 默认语言
export const langDefault = process.env.LANG_ENV ?? 'zh-CN';
const extraLanguage = getExtraLanguage();
const mergeCNLanguage = getMergeCNLanguage();
const mergeENLanguage = getMergeENLanguage();

const i18n = createI18n({
  locale: extraLanguage ? 'extra' : langDefault, //默认显示的语言
  messages: {
    ...messages,
    extra: extraLanguage || {},
  },
});

if (extraLanguage) {
  console.log('Found extra language');
}

if (mergeCNLanguage) {
  console.log('Found cn merge language');
  i18n.global.mergeLocaleMessage('zh-CN', mergeCNLanguage);
}

if (mergeENLanguage) {
  console.log('Found en merge language');
  i18n.global.mergeLocaleMessage('en-US', mergeENLanguage);
}

export default i18n;
globalThis.i18n = i18n;
