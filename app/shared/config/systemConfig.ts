import { KF_CONFIG_PATH, KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { readJsonSync, outputJsonSync } from "__gUtils/fileUtils";

const isEnglish = process.env.LANG_ENV === 'en';

const enum SystemConfigChildItemTypeEnum {
    Select = 'select',
    Bool = 'bool',
    Str = 'str',
    Password = 'password',
    Int = 'int',
    Float = 'float',
    File = 'file',
    Process = 'process',
    Sources = 'sources',
    Table = 'table',
}

const enum SystemConfigTypeEnum {
    SystemConfig = 'systemConfig',
    SystemTradingConfig = 'systemTradingConfig'
}

export interface SystemConfigItem {
    key: string;
    name: string;
    config: Array<SystemConfigChildNormalItem>;
    cli?: Boolean;
    process?: Boolean;
    configType?: string;
}

export interface SystemConfigChildNormalItem {
    key: string;
    name: string;
    tip: string;
    type: SystemConfigChildItemTypeEnum,
    default?: any;
    cli?: boolean;
    required?: boolean;
    data?: SystemConfigChildSelectItemData[] // select'
    args?: SystemConfigChildArgsItemData[] // process
    target?: string; // process + table
    unique_key?: string; // table
    row?: SystemConfigChildTableRowItemData[]; // table
    parentKey?: string; //for bussiness use, extra
}

interface SytemConfigChildProcessItem {
    key: string;
    name: string;
    tip: string;
    type: SystemConfigChildItemTypeEnum,
    args: SystemConfigChildArgsItemData[] // process
    target: string; // process
    parentKey: string; //for bussiness use, extra
}

interface SystemConfigChildTableRowItemData {
    key: string;
    name: string;
    type: SystemConfigChildItemTypeEnum;
    default?: any;
    data?: SystemConfigChildSelectItemData[];
    cellWidth?: Number
}


export const getSystemConfig = (): { [propName: string]: SystemConfigItem } => ({
    "performance": {
        "key": "performance",
        "name": isEnglish ? "Performance" : "性能",
        "cli": true,
        "config": [
            {
                "key": "rocket",
                "name": isEnglish ? "Open Rocket Model" : "开启极速模式",
                "cli": true,
                "tip": isEnglish ? "Use CPU 100%, restart is required" : "开启极速模式会使 CPU 达到100%，开启后请重启功夫交易系统",
                "default": false,
                "type": SystemConfigChildItemTypeEnum.Bool,
                "required": true
            },
            {
                "key": "bypassQuote",
                "name": isEnglish ? "Close Quote Listener" : "关闭Quote监听",
                "cli": true,
                "tip": isEnglish ? "Close Quote Listener" : "在订阅一定数量（上千）支Ticker时，由行情推送触发pnl计算会导致性能问题，需关闭后（打开下方开关）重启功夫交易系统（带来的影响为在发生交易的时候才会触发pnl更新）",
                "default": false,
                "type": SystemConfigChildItemTypeEnum.Bool,
                "required": true
            },
           
            
        ]
    },
    "strategy": {
        "key": "strategy",
        "name": isEnglish ? "Strategy" : "策略",
        "cli": true,
        "config": [
            {
                "key": 'pythonPath',
                "name": isEnglish ? 'Select Local Python Path' : '选择本地 Python 路径',
                "cli": true,
                "tip": isEnglish ? 'local python path is required to be selected, and kungfu*.whl should be installed in this path' : '只有选择了本地python路径，且kungfu*.whl 通过这个python安装，Kungfu才能使用本地python',
                "default": '',
                "type": SystemConfigChildItemTypeEnum.File,
                "required": true
            },
            {
                "key": "python",
                "name": isEnglish ? "Use Local Python" : "使用本地python",
                "cli": true,
                "tip": isEnglish ? `Pip3 install kungfu*.whl, local python require ${python_version}` : `使用本地python启动策略，需要 pip3 install kungfu*.whl，本地 python3 版本需为 ${python_version}，开启后需重启策略`,
                "default": false,
                "type": SystemConfigChildItemTypeEnum.Bool,
                "required": true
            }
        ]
    },

    "log": {
        "key": "log",
        "name": isEnglish ? "Log" : "日志",
        "cli": true,
        "config": [
            {
                "key": "level",
                "name": isEnglish ? "Log level" : "级别",
                "tip": isEnglish ? "For all log" : "对系统内所有日志级别的设置",
                "type": SystemConfigChildItemTypeEnum.Select,
                "cli": true,
                "data": [
                    { "value": "-l trace", "name": "TRACE" },
                    { "value": "-l debug", "name": "DEBUG" },
                    { "value": "-l info", "name": "INFO" },
                    { "value": "-l warning", "name": "WARN" },
                    { "value":"-l error", "name": "ERROR" },
                    { "value":"-l critical", "name": "CRITICAL" }
                ],
                "required": true
            }
        ]
    },

    "code": {
        "key": "code",
        "name": "编辑器",
        "cli": false,
        "config": [
            {
                "key": "tabSpaceType",
                "name": "缩进类别",
                "tip": "功夫编辑器缩进类别",
                "type": SystemConfigChildItemTypeEnum.Select,
                "data": [
                    {"value": "Spaces", "name": "Spaces"},
                    {"value": "Tabs", "name": "Tabs"}
                ]
            },
            {
                "key": "tabSpaceSize",
                "name": "缩进大小",
                "tip": "功夫编辑器缩进大小（空格）",
                "type": SystemConfigChildItemTypeEnum.Select,
                "data": [
                    {"value": 2, "name": 2},
                    {"value": 4, "name": 4}
                ]
            }
        ]
    }
})

export const getSystemTradingConfig = (): { [propName: string]: SystemConfigItem }  => ({
    "bar": {
        "key": "bar",
        "name": "BAR",
        "cli": true,
        "process": true,
        "config": [
          {
            "key": "open",
            "name": isEnglish ? "Open" : "开启",
            "tip": isEnglish ? "Open for bar calculation" : "开启计算Bar数据进程",
            "type": SystemConfigChildItemTypeEnum.Process,
            "target": "bar",
            "args": [ 
                {"key": "-s", "value": "source" }, 
                {"key": "--time-interval", "value": "time_interval"}],
          },
          {
            "key": "source",
            "name": isEnglish ? "Source support" : "支持柜台",
            "cli": true,
            "tip": isEnglish ? "" : "选择支持订阅Bar功能的柜台名称，不能为空",
            "type": SystemConfigChildItemTypeEnum.Sources,
            "required": true
          },
          {
            "key": "time_interval",
            "name": isEnglish ? "Time interval" : "频率",
            "cli": true,
            "tip": isEnglish ? "" : "选择Bar的频率，不能为空",
            "type": SystemConfigChildItemTypeEnum.Select,
            "data": [
              { "value": "30s", "name": "30s" },
              { "value": "1m", "name": "1min" },
              { "value": "2m", "name": "2min" },
              { "value": "3m", "name": "3min" },
              { "value": "4m", "name": "4min" },
              { "value": "5m", "name": "5min" },
              { "value": "10m", "name": "10min" },
              { "value": "15m", "name": "15min" }
            ],
            "required": true
          }
        ]
    },
    
    "commission": {
      "key": "commission",
      "name": "手续费",
      "cli": false,
      "config": [
        {
          "key": "future",
          "name": "期货",
          "tip": "仅为期货手续费设置，股票手续费会自动读取，无需设置，设置完成后需重启Kungfu",
          "type": SystemConfigChildItemTypeEnum.Table,
          "target": "commission",
          "unique_key": "row_id",
          "row": [
            {
              "key": "product_id",
              "name": "标的品种",
              "type": SystemConfigChildItemTypeEnum.Str,
              "default": "",
            },
            {
              "key": "exchange_id",
              "name": "交易所",
              "type": SystemConfigChildItemTypeEnum.Str,
              "default": ""

            },
            {
              "key": "mode",
              "name": "类型",
              "type": SystemConfigChildItemTypeEnum.Select,
              "default": "",
              "data": [
                { "value": "0", "name": "按金额" },
                { "value": "1", "name": "按手数" }
              ]
            }, 
            {
              "key": "open_ratio",
              "name": "开仓手续费",
              "type": SystemConfigChildItemTypeEnum.Float,
              "default": 0
            },
            {
              "key": "close_ratio",
              "name": "平仓手续费",
              "type": SystemConfigChildItemTypeEnum.Float,
              "default": 0
            },
            {
              "key": "close_today_ratio",
              "name": "平今手续费",
              "type": SystemConfigChildItemTypeEnum.Float,
              "default": 0
            }
          ]
        }
      ]
    }
});

interface CustomProcessData {
    [propName: string]: SytemConfigChildProcessItem
}

const convertToProcessItem = (configItem: SystemConfigChildNormalItem): SytemConfigChildProcessItem => {
    return {
        key: configItem.key,
        name: configItem.name,
        tip: configItem.tip,
        type: configItem.type,
        args: configItem.args || [], // process
        target: configItem.target || '', // process
        parentKey: configItem.parentKey || ''
    }
}

export const buildCustomProcessConfig = (): CustomProcessData => {
    const systemConfigSetting = {
        ...getSystemConfig(),
        ...getSystemTradingConfig()
    };
    const customProcessParentList: SystemConfigItem[] = Object.values(systemConfigSetting || {}).filter((config: SystemConfigItem) => config.process)
    const customProcessList = customProcessParentList
        .map((item: SystemConfigItem): SystemConfigChildNormalItem[] => {
            const itemKey = item.key;
            return item.config.filter((itemConfig) => {
                itemConfig.parentKey = itemKey;
                return itemConfig.type === SystemConfigChildItemTypeEnum.Process;
            })
        })
        .reduce((config1, config2): SystemConfigChildNormalItem[] => {
            return [...config1, ...config2]
        })

    let customProcessData: CustomProcessData = {};
    customProcessList.forEach((configItem) => {
        customProcessData[configItem.target || ''] = convertToProcessItem(configItem)
    })
    return customProcessData
}

export const buildSystemConfig = () => {
    const kfSystemConfig = readJsonSync(KF_CONFIG_PATH) || {};
    const kfSystemTradingConfig = readJsonSync(KF_TARADING_CONFIG_PATH) || {};
    return {
        system: {
            key: "system",
            name: "系统设置",
            config: getSystemConfig(),
            value: kfSystemConfig,
            outputPath: KF_CONFIG_PATH,
            type: "json"
        },
        trading: {
            key: "trading",
            name: "交易设置",
            config: getSystemTradingConfig(),
            value: kfSystemTradingConfig,
            outputPath: KF_TARADING_CONFIG_PATH,
            type: "json"
        }
    }
}

export const mergeSystemConfigTogether = () => {
    let systemConfig = getSystemConfig();
    let systemTradingConfig = getSystemTradingConfig();

    Object.values(systemConfig).forEach((systemConfigItem: SystemConfigItem) => {
        if(!systemConfigItem.cli) delete systemConfig[systemConfigItem.key]
        systemConfigItem.configType = SystemConfigTypeEnum.SystemConfig
    });
    Object.values(systemTradingConfig).forEach((systemConfigItem: SystemConfigItem) => {
        if(!systemConfigItem.cli) delete systemTradingConfig[systemConfigItem.key]
        systemConfigItem.configType = SystemConfigTypeEnum.SystemTradingConfig
    });

    return {
        ...systemConfig,
        ...systemTradingConfig
    }
}

export const getValueByConfigItem = (configType: string, configItemKey: string) => {
    const kfSystemConfig = readJsonSync(KF_CONFIG_PATH) || {};
    const kfSystemTradingConfig = readJsonSync(KF_TARADING_CONFIG_PATH) || {};
    const kfConfigData: { [propName: string]: { [propName: string]: SystemConfigItem }}= {
        'systemConfig': kfSystemConfig,
        'systemTradingConfig': kfSystemTradingConfig
    }
    const targetConfigData = ((kfConfigData[configType] || {})[configItemKey] || {})
    if(Object.keys(targetConfigData).length === 0) throw new Error('Target value has not been initialized!')
    return targetConfigData
}

export const setValueByConfigItem = (configType: string, configItemKey: string, configChildItemData: StringToAnyObject) => {
    if(configType === 'systemConfig') {
        let kfSystemConfig: StringToAnyObject = readJsonSync(KF_CONFIG_PATH) || {};
        kfSystemConfig[configItemKey] = {
            ...kfSystemConfig[configItemKey],
            ...configChildItemData
        }
        outputJsonSync(KF_CONFIG_PATH, kfSystemConfig)
    } else if(configType === 'systemTradingConfig') {
        const kfSystemTradingConfig: StringToAnyObject = readJsonSync(KF_TARADING_CONFIG_PATH) || {};
        kfSystemTradingConfig[configItemKey] = {
            ...kfSystemTradingConfig[configItemKey],
            ...configChildItemData
        }
        outputJsonSync(KF_TARADING_CONFIG_PATH, kfSystemTradingConfig)
    }
}

  