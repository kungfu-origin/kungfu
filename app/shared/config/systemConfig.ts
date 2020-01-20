import { KF_CONFIG_PATH, KF_TARADING_CONFIG_PATH } from '__gConfig/pathConfig';
import { readJsonSync } from "__gUtils/fileUtils";


export const getSystemConfig = () => ({
    "performance": {
        "key": "performance",
        "name": "性能",
        "cli": true,
        "config": [
            {
                "key": "rocket",
                "name": "开启极速模式",
                "cli": true,
                "tip": "开启极速模式会使 CPU 达到100%，开启后请重启 Kungfu",
                "default": false,
                "type": "bool"
            }
        ]
    },
    "strategy": {
        "key": "strategy",
        "name": "策略",
        "cli": true,
        "config": [
            {
                "key": "python",
                "name": "使用本地python",
                "cli": true,
                "tip": `使用本地python启动策略，需要 pip3 install kungfu*.whl，本地 python3 版本需为 ${python_version}，开启后需重启策略`,
                "default": false,
                "type": "bool"
            }
        ]
    },

    "log": {
        "key": "log",
        "name": "日志",
        "cli": true,
        "config": [
            {
                "key": "level",
                "name": "级别",
                "tip": "对系统内所有日志级别的设置",
                "type": "select",
                "data": [
                    { "value": "-l trace", "name": "TRACE" },
                    { "value": "-l debug", "name": "DEBUG" },
                    { "value": "-l info", "name": "INFO" },
                    { "value": "-l warning", "name": "WARN" },
                    { "value":"-l error", "name": "ERROR" },
                    { "value":"-l critical", "name": "CRITICAL" }
                ]
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
                "tip": "kungfu 编辑器缩进类别",
                "type": "select",
                "data": [
                    {"value": "Spaces", "name": "Spaces"},
                    {"value": "Tabs", "name": "Tabs"}
                ]
            },
            {
                "key": "tabSpaceSize",
                "name": "缩进大小",
                "tip": "kungfu 编辑器缩进大小（空格）",
                "type": "select",
                "data": [
                    {"value": 2, "name": 2},
                    {"value": 4, "name": 4}
                ]
            }
        ]
    }
})


export const getSystemTradingConfig = () => ({
    "bar": {
        "key": "bar",
        "name": "BAR",
        "cli": true,
        "process": true,
        "config": [
          {
            "key": "open",
            "name": "开启",
            "tip": "开启计算Bar数据进程",
            "type": "process",
            "target": "bar",
            "start": "startBar",
            "args": [ 
                {"key": "-s", "value": "source" }, 
                {"key": "--time-interval", "value": "time_interval"}],
            "log_path": "bar"
          },
          {
            "key": "source",
            "name": "支持柜台",
            "cli": true,
            "tip": "选择支持订阅Bar功能的柜台名称，不能为空",
            "type": "sources"
          },
          {
            "key": "time_interval",
            "name": "频率",
            "cli": true,
            "tip": "选择Bar的频率，不能为空",
            "type": "select",
            "data": [
              { "value": "30s", "name": "30s" },
              { "value": "1m", "name": "1min" },
              { "value": "2m", "name": "2min" },
              { "value": "3m", "name": "3min" },
              { "value": "4m", "name": "4min" },
              { "value": "5m", "name": "5min" },
              { "value": "10m", "name": "10min" },
              { "value": "15m", "name": "15min" }
            ]
          }
        ]
    },
    
    "comission": {
      "key": "commission_setting",
      "name": "手续费",
      "cli": false,
      "config": [
        {
          "key": "future",
          "name": "期货",
          "tip": "仅为期货手续费设置，股票手续费会自动读取，无需设置",
          "type": "table",
          "target": "commission",
          "unique_key": "row_id",
          "row": [
            {
              "key": "product_id",
              "name": "产品",
              "type": "str",
              "default": ""
            },
            {
              "key": "exchange_id",
              "name": "交易所",
              "type": "str",
              "default": ""

            },
            {
              "key": "mode",
              "name": "类型",
              "type": "select",
              "default": "",
              "data": [
                { "value": "0", "name": "按金额" },
                { "value": "1", "name": "按手数" }
              ]
            }, 
            {
              "key": "open_ratio",
              "name": "开仓手续费",
              "type": "float",
              "default": 0
            },
            {
              "key": "close_ratio",
              "name": "平仓手续费",
              "type": "float",
              "default": 0
            },
            {
              "key": "close_today_ratio",
              "name": "平今手续费",
              "type": "float",
              "default": 0
            }
          ]
        }
      ]
    }
});

export const buildSystemConfig = () => {
    const kfSystemConfig = readJsonSync(KF_CONFIG_PATH) || {};
    const kfTradingConfig = readJsonSync(KF_TARADING_CONFIG_PATH) || {};

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
            value: kfTradingConfig,
            outputPath: KF_TARADING_CONFIG_PATH,
            type: "json"
        }
    }
}
  