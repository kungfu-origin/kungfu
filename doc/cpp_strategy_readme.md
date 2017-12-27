# 一、快速上手

功夫开源交易系统框架介绍请关注[硅商冲击](https://zhuanlan.zhihu.com/silicontrader)

系统使用方法请查看文档[功夫交易系统使用文档](https://github.com/lqyhost/kungfu/blob/master/doc/kungfu_document_cn.md)

python策略api文档请查看[C++策略api文档](https://github.com/lqyhost/kungfu/blob/master/doc/cpp_strategy_document_cn.md)

## C++策略demo编译与执行

  官方提供的docker环境中已经集成了cmake编译工具，CMake文件可以参考源代码目录下wingchun/strategy/cpp_demo/CMakeLists.txt。
 
 用户可以通过编译运行开源代码 wingchun/strategy/cpp_demo 目录下的策略来熟悉C++的策略的编译与执行， 由于cpp_demo默认不编译，如需使用，需要手动编译。编译和运行方法如下：
 
 ```
 $ cd cpp_demo
 $ mkdir cpp_build
 $ cd cpp_build
 $ cmake ../
 $ make
 $ ./kungfu_strategy_demo
 ``` 
 
##  头文件

 编写C++策略需要参考的的头文件位于如下两个目录中：
 
 策略基类和功能对象基类头文件位于 /opt/kungfu/master/include/ 目录下：
 
* IWCStrategy.h 策略基类定义
 
* WCDataWrapper.h 策略数据功能对象data定义
 
* WCStrategyUtil.h 为策略执行功能对象util定义
 
 数据结构和常量定义的头文件位于/opt/kungfu/master/include/longfist/目录下：
 
* LFConstants.h 交易常量定义
 
* LFDataStruct.h 数据结构定义
 
* sys_messages.h 系统消息类别定义
  
## C++策略基类IWCStrategy

C++策略通过继承IWCStrategy获得信号回调函数，在回调函数中可以实现自己的业务逻辑

```
#include "IWCStrategy.h"

class Strategy: public IWCStrategy
{
protected:
    bool td_connected;
    int rid;
public:
    virtual void init();
    virtual void on_market_data(const LFMarketDataField* data, short source, long rcv_time);
    virtual void on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time);
    virtual void on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time);
    virtual void on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr);

public:
    Strategy(const string& name);
};
```

## 策略的执行入口

策略执行的入口为main函数，执行过程如下：

创建策略对象并命名

运行初始化函数

启动策略进程

阻塞并保持策略运行

```
int main(int argc, const char* argv[])
{
    Strategy str(string("cpp_test"));
    str.init();
    str.start();
    str.block();
    return 0;
}
```

# 二、 策略常用函数接口

## 策略回调函数

策略回调函数包括行情回调，交易回调和系统回调，策略通过回调获取行情和交易返回信息。详细的回调函数信息可参考[C++策略api文档](https://github.com/lqyhost/kungfu/blob/master/doc/cpp_strategy_document_cn.md)

行情回调函数在策略订阅行情以后，　收到行情时回调

交易回调函数在下单后，从交易所收到下单回报或成交回报时时回调

系统回调在一些系统内部逻辑运行时被回调，比如系统内有一个换天逻辑，会在这个时间将系统持仓中今仓转换为昨仓，此时
on_switch_day函数就会被调用，在需要持续运行的策略中，如果用户需要在交易日切换时对内部数据进行一些处理的话，可以在其中予以实现。 

```
/* market data call back function */
on_market_data(const LFMarketDataField* data, short source, long rcv_time);
on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time);

/* trade data call back function */
on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time);
on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time);
on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr);
on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time);

/* system utilities call back function*/
on_switch_day(long rcv_time);
on_time(long cur_time);
on_td_login(bool ready, const json& js, short source);
```


## 策略功能接口函数

功夫交易系统C++策略通过IWCStrategy中保护成员data和utils的函数实现功能，详细的功能接口函数功能可参考[C++策略api文档](https://github.com/lqyhost/kungfu/blob/master/doc/cpp_strategy_document_cn.md)


data为策略数据功能成员对象，其定义在/opt/kungfu/master/include/WCStrategyUtil.h。如下示例为通过data成员为策略添加行情和交易数据接入

```
data->add_market_data(SOURCE.CTP);
data->add_register_td(SOURCE.CTP);
```

utils为策略执行功能成员对象，其定义在/opt/kungfu/master/include/WCDataWrapper.h。如下示例为通过util成员实现下限价单：

```
rid = util->insert_limit_order(SOURCE.CTP, M_TICKER, M_EXCHANGE,
                               md->LowerLimitPrice, signal.trade_size,
                               LF_CHAR_Sell, LF_CHAR_CloseToday);
```

