import '../config/homePathConfig';
import { createI18n } from 'vue-i18n';
import zh_CN from './zh-CN';
import en_US from './en-US';
import path from 'path';
import fse from 'fs-extra';
import { KF_HOME_BASE_DIR_RESOLVE } from '../config/homePathConfig';

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
    return null;
  } else {
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
let settingLanguage;
const extraLanguage = getExtraLanguage();
const mergeCNLanguage = getMergeCNLanguage();
const mergeENLanguage = getMergeENLanguage();
const kfConfigPath = path.join(
  KF_HOME_BASE_DIR_RESOLVE,
  'home',
  'config',
  'kfConfig.json',
);

if (fse.existsSync(kfConfigPath)) {
  const globalSettingJson = fse.readJSONSync(kfConfigPath) as Record<
    string,
    Record<string, KungfuApi.KfConfigValue>
  >; // 不直接使用 getKfGlobalSettingsValue 和 KF_CONFIG_PATH 是因为会形成循环引用，会报错
  settingLanguage = globalSettingJson?.system?.language ?? langDefault;
}

// 语言库
const messages = {
  'zh-CN': zh_CN,
  'en-US': en_US,
  extra: extraLanguage || {},
};

export const languageList = [
  { value: 'zh-CN', label: '简体中文' },
  { value: 'en-US', label: 'English' },
  ...(extraLanguage
    ? [
        {
          value: 'extra',
          label: '自定义语言',
        },
      ]
    : []),
];

const locale = settingLanguage || (extraLanguage ? 'extra' : langDefault); //默认显示的语言

const i18n =
  globalThis.i18n ??
  createI18n({
    locale,
    messages,
    silentTranslationWarn: true,
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

export const useLanguage = () => {
  const buildExtLangKey = (extKey: string, extraKeys?: string[] | string) => {
    if (!extraKeys) return extKey;

    return [
      extKey,
      ...(Array.isArray(extraKeys) ? extraKeys : [extraKeys]),
    ].join('.');
  };

  const isLanguageKeyAvailable = (keysStr: string) => {
    if (!keysStr) return false;

    let result: string | undefined;
    const languageData = i18n.global.messages[i18n.global.locale];

    const keysList = keysStr.split('.');
    if (keysList.length) {
      result = keysList.reduce<string | undefined>(
        (res, key) => res?.[key],
        languageData,
      );
    }

    return typeof result === 'string';
  };

  return {
    buildExtLangKey,
    isLanguageKeyAvailable,
  };
};

export default i18n;
if (!globalThis.i18n) globalThis.i18n = i18n;
