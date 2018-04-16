# C++ 策略开发快速入门


参考策略的 C++ API，请[查看](cpp_strategy_api.md)

## C++ 策略 demo 编译与执行

  官方提供的 docker 环境中已经集成了 cmake 编译工具。
 
  用户可以通过编译运行开源代码 wingchun/strategy/cpp_demo 目录下的策略来熟悉C++的策略的编译与执行，编译和运行方法如下：
 
 ```
 $ cd cpp_demo
 $ mkdir cpp_build
 $ cd cpp_build
 $ cmake ../
 $ make
 $ ./kungfu_strategy_demo
 ``` 
 
##  头文件

 编写 C++ 策略需要参考的的头文件位于如下两个目录中：
 
 策略基类和功能对象基类头文件位于 /opt/kungfu/master/include/ 目录下：
 
* IWCStrategy.h 策略基类定义
 
* WCDataWrapper.h 策略功能对象data定义
 
* WCStrategyUtil.h 策略功能对象util定义
 
 数据结构和常量定义的头文件位于 /opt/kungfu/master/include/longfist/ 目录下：
 
* LFConstants.h 交易常量定义
 
* LFDataStruct.h 数据结构定义
 
* sys_messages.h 系统消息类别定义
  
## C++策略基类IWCStrategy

C++策略通过继承 IWCStrategy 继承信息回调函数，在回调函数中可以实现的策略逻辑

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

策略执行的入口为 main 函数，执行过程如下：

* 建策略对象并命名
* 运行初始化函数
* 启动策略进程
* 阻塞并保持策略运行

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
