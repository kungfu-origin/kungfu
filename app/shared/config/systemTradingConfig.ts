export const getSystemTradingConfig = () => ({
    "bar": {
        "key": "bar",
        "name": "BAR",
        "config": [
          {
            "key": "open",
            "name": "开启",
            "tip": "开启计算Bar数据进程",
            "type": "process",
            "target": "bar",
            "start": "startBar",
            "args": ["source", "time_interval"],
            "log_path": "bar"
          },
          {
            "key": "source",
            "name": "支持柜台",
            "tip": "选择支持订阅Bar功能的柜台名称，不能为空",
            "type": "sources"
          },
          {
            "key": "time_interval",
            "name": "频率",
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
})
  