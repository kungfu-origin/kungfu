import path from 'path';
import fse from 'fs-extra';
import inquirer from 'inquirer';
import colors from 'colors';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  getAvailCliDaemonList,
  getIdByKfLocation,
  getKfCliExtensionConfig,
  getProcessIdByKfLocation,
  initFormStateByConfig,
  loopToRunProcess,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import {
  BrokerStateStatus,
  Pm2ProcessStatus,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { PromptInputType, PromptQuestion } from '../../typings';
import { startExtDaemon } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { Proc } from 'pm2';
import { globalState } from '../actions/globalState';
import { program } from 'commander';

export const parseToString = (
  targetList: (string | number)[],
  columnWidth: (string | number)[],
  pad = 2,
) => {
  return targetList
    .map((item: string | number, i: number) => {
      if (item + '' === '0') item = '0';
      item = (item || '').toString();

      const lw = item
        .replace(/\u001b\[1m/g, '')
        .replace(/\u001b\[22m/g, '')
        .replace(/\u001b\[31m/g, '')
        .replace(/\u001b\[32m/g, '')
        .replace(/\u001b\[33m/g, '')
        .replace(/\u001b\[34m/g, '')
        .replace(/\u001b\[35m/g, '')
        .replace(/\u001b\[36m/g, '')
        .replace(/\u001b\[37m/g, '')
        .replace(/\u001b\[38m/g, '')
        .replace(/\u001b\[39m/g, '')
        .replace(/\u001b\[45m/g, '')
        .replace(/\u001b\[49m/g, '');

      const len = lw.length;
      const colWidth: number | string = columnWidth[i] || 0;
      if (colWidth === 'auto') return item;
      const spaceLength = +colWidth - len;
      if (spaceLength < 0) return lw.slice(0, +colWidth);
      else if (spaceLength === 0) return item;
      else return item + new Array(spaceLength + 1).join(' ');
    })
    .join(new Array(pad + 2).join(' '));
};

export const getKfCategoryFromString = (
  typeString: string,
): KfCategoryTypes => {
  const isTd = typeString.toLocaleLowerCase().includes('td');
  const isMd = typeString.toLocaleLowerCase().includes('md');
  const isStrategy = typeString.toLocaleLowerCase().includes('strategy');

  if (isTd) return 'td';
  else if (isMd) return 'md';
  else if (isStrategy) return 'strategy';
  else return 'system';
};

export const parseExtDataList = (
  extList: KungfuApi.KfExtConfig[],
): string[] => {
  return extList.map((ext: KungfuApi.KfExtConfig) => {
    const isArray = typeof ext.type === 'object';
    const type = isArray ? (ext.type || []).join(' ') : ext.type || '';
    return [ext.name, ext.key, type].join('    ');
  });
};

export const getPromptQuestionsBySettings = (
  settings: KungfuApi.KfConfigItem[],
  initValue?: Record<string, KungfuApi.KfConfigValue>,
): Promise<KungfuApi.KfConfigValue> => {
  const formState = initFormStateByConfig(settings, initValue || {});
  const questions = settings.map((item) =>
    buildQuestionByKfConfigItem(item, formState[item.key], !!initValue),
  );

  return inquirer
    .prompt(questions)
    .then((answers: Record<string, KungfuApi.KfConfigValue>) => {
      return trimAnswers(answers);
    });
};

export const getQuestionInputType = (
  originType: KungfuApi.KfConfigItemSupportedTypes,
): PromptInputType => {
  switch (originType) {
    case 'str':
    case 'password':
      return 'input';
    case 'int':
    case 'float':
      return 'number';
    case 'select':
    case 'radio':
    case 'side':
    case 'offset':
    case 'direction':
    case 'priceType':
    case 'hedgeFlag':
    case 'volumeCondition':
    case 'timeCondition':
    case 'commissionMode':
    case 'instrumentType':
    case 'td':
    case 'md':
    case 'strategy':
      return 'list';
    case 'bool':
      return 'confirm';
    case 'file':
    case 'folder':
      return 'path';
    default:
      return 'input';
  }
};

export const renderSelect = (configItem: KungfuApi.KfConfigItem) => {
  if (configItem.type === 'select' || configItem.type === 'radio')
    return `(${(configItem.options || configItem.data || [])
      .map((item) => item.value || '')
      .join('|')})`;
  else return '';
};

export const buildQuestionByKfConfigItem = (
  configItem: KungfuApi.KfConfigItem,
  value: KungfuApi.KfConfigValue | undefined,
  isUpdate = false,
) => {
  const { key, type } = configItem;
  const targetType = getQuestionInputType(type);
  const questions: PromptQuestion = {
    type: targetType,
    name: key,
    choices:
      targetType === 'list'
        ? (configItem.options || configItem.data || []).map(
            (item) => item.value,
          )
        : [],
    message: `${isUpdate ? 'Update' : 'Enter'} ${key} ${renderSelect(
      configItem,
    )} ${configItem.tip ? '(' + configItem.tip + ')' : ''}`,

    validate: async (value: KungfuApi.KfConfigValue) => {
      if (configItem.required && value.toString() === '') {
        return new Error('Required');
      }

      return true;
    },
  };

  if (value !== undefined && value !== '' && value !== 0) {
    questions.default = value;
  }

  return questions;
};

export const trimAnswers = (answers: Record<string, string | number>) => {
  Object.keys(answers || {}).forEach((key: string) => {
    if (typeof answers[key] === 'string') {
      answers[key] = (answers[key] as string).trim();
    }
  });
  return answers;
};

export const buildObjectFromKfConfigArray = (
  kfConfigs: KungfuApi.KfConfig[],
) => {
  return kfConfigs.reduce((data, item: KungfuApi.KfConfig) => {
    data[getIdByKfLocation(item)] = item;
    return data;
  }, {} as Record<string, KungfuApi.KfConfig>);
};

export const getKfLocation = (
  type: KfCategoryTypes,
  targetId: string,
): KungfuApi.KfLocation => {
  if (type === 'md') {
    return {
      category: 'md',
      group: targetId,
      name: targetId,
      mode: 'live',
    };
  } else if (type === 'td') {
    return {
      category: 'td',
      group: (targetId || '').parseSourceAccountId().source,
      name: (targetId || '').parseSourceAccountId().id,
      mode: 'live',
    };
  } else if (type === 'strategy') {
    return {
      category: 'strategy',
      group: 'default',
      name: targetId,
      mode: 'live',
    };
  } else {
    throw new Error(`Unsupported update category ${type}`);
  }
};

export const selectTargetKfConfig = async (
  noMd = false,
): Promise<KungfuApi.KfConfig | null> => {
  const { md, td, strategy } = await getAllKfConfigOriginData();

  const mdTdStrategyList = [
    ...(noMd
      ? []
      : md.map((item) =>
          parseToString(
            [colors.yellow('md'), getIdByKfLocation(item)],
            [8, 'auto'],
            1,
          ),
        )),
    ...td.map((item) =>
      parseToString(
        [colors.blue('td'), getIdByKfLocation(item)],
        [8, 'auto'],
        1,
      ),
    ),
    ...strategy.map((item) =>
      parseToString(
        [colors.cyan('strategy'), getIdByKfLocation(item)],
        [8, 'auto'],
        1,
      ),
    ),
  ];

  const answers: { process: string } = await inquirer.prompt([
    {
      type: 'autocomplete',
      name: 'process',
      message: 'Select targeted md / td / strategy  ',
      source: async (answersSoFar: { process: string }, input: string) => {
        input = input || '';
        return mdTdStrategyList.filter((s: string): boolean =>
          s.includes(input),
        );
      },
    },
  ]);

  const processes = answers.process;
  const splits = processes.split(' ');
  const targetType = splits[0].trim();
  const targetId = splits[splits.length - 1].trim();
  const type = getKfCategoryFromString(targetType);
  const kfLocation = getKfLocation(type, targetId);
  const processId = getProcessIdByKfLocation(kfLocation);
  const searchList = [...md, ...td, ...strategy];
  const targetIndex = searchList.findIndex((item: KungfuApi.KfConfig) => {
    const id = getProcessIdByKfLocation(item);
    if (id === processId) {
      return true;
    }

    return false;
  });

  if (targetIndex === -1) {
    return null;
  } else {
    return searchList[targetIndex];
  }
};

export const dealStatus = (status: string): string => {
  if (status === '--') return status;
  if (!Pm2ProcessStatus[status] && !BrokerStateStatus[status]) return status;
  const name: string =
    BrokerStateStatus[status]?.name || Pm2ProcessStatus[status]?.name || '';
  const level: number =
    BrokerStateStatus[status]?.level || Pm2ProcessStatus[status]?.level || 0;
  if (level >= 1) return colors.green(name);
  else if (level == 0) return colors.white(name);
  else if (level < 0) return colors.red(name);
  else return status;
};

export const dealMemory = (mem: number): string => {
  if (!mem) {
    return '--';
  }
  return Number((mem || 0) / (1024 * 1024)).toFixed(0) + 'MB';
};

export const dealProcessName = (name: string) => {
  return name ? name.split('_').at(-1) : null;
};

export const calcHeaderWidth = (
  target: string[],
  wish: (string | number)[],
) => {
  wish = wish || [];
  return target.map((t: string, i) => {
    if (wish[i] === 'auto') return wish[i];
    if (t.length < (wish[i] || 0)) return wish[i];
    else return t.length;
  });
};

export const getCategoryName = (category: KfCategoryTypes) => {
  if (category === 'md') {
    return colors.yellow('Md');
  } else if (category === 'td') {
    return colors.cyan('td');
  } else if (category === 'strategy') {
    return colors.blue('Strat');
  } else if (category === 'daemon') {
    return colors.green('Daem');
  } else {
    return colors.bgMagenta('Sys');
  }
};

export const colorNum = (num: number | string): string => {
  if (+num > 0) {
    return colors.red(num.toString());
  } else if (+num === 0) {
    return num.toString();
  } else {
    return colors.green(num.toString());
  }
};

export const getPadSpace = (num: number) => {
  return new Array(num + 1).join(' ');
};

export const isObject = (val) => {
  return Object.prototype.toString.call(val) === '[object Object]';
};

export const startAllExtDaemons = async () => {
  const availDaemons = await getAvailCliDaemonList();
  return loopToRunProcess<void | Proc>(
    availDaemons.map((item) => {
      return () =>
        startExtDaemon(getProcessIdByKfLocation(item), item.cwd, item.script)
          .then((res) => {
            return res;
          })
          .catch((err) => console.error(err));
    }),
  );
};

interface KfExtModule {
  install: (gs: unknown, gt: typeof globalThis) => void;
}

const indexUse = (ext: KfExtModule) => {
  const { install } = ext;
  if (install) {
    install(program);
  }
};

const dzxyUse = (ext: KfExtModule) => {
  const { install } = ext;
  if (install) {
    install(globalState, globalThis);
  }
};

export const useAllExtScript = () => {
  return getKfCliExtensionConfig()
    .then((allConfigs) =>
      Promise.all(
        Object.values(allConfigs).map((config) => {
          const { extPath, script } = config;
          const scriptPath = path.join(extPath, script);
          if (script && fse.pathExistsSync(scriptPath)) {
            return <Record<string, KfExtModule>>(
              __non_webpack_require__(scriptPath)
            );
          }

          return null;
        }),
      ),
    )
    .then((allExtModules) => {
      allExtModules.forEach((extModule) => {
        if (extModule) {
          dzxyUse(extModule['default']);
        }
      });
    });
};

export const useAllExtComponentByPosition = async (
  curPosition: 'index' | 'dzxy',
) => {
  const allConfigs = await getKfCliExtensionConfig();
  for (const config of Object.values(allConfigs)) {
    const { components, extPath } = config;
    if (components) {
      for (const key of Object.keys(components)) {
        const { entry, position } = components[key];

        if (position !== curPosition) continue;

        const componentPath = path.join(extPath, entry);
        if (entry && fse.pathExistsSync(componentPath)) {
          const extModule = <Record<string, KfExtModule>>(
            await __non_webpack_require__(componentPath)
          );
          if (extModule) {
            if (curPosition === 'index') {
              indexUse(extModule['default']);
            } else if (curPosition === 'dzxy') {
              dzxyUse(extModule['default']);
            }
          }
        }
      }
    }
  }
};
