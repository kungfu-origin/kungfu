Intro 简介
========

功夫是 [Taurus.ai](http://taurus.ai) 团队专为量化交易者设计的低延迟交易执行系统。功夫想要解决两个核心问题：
* 交易数据实时存储 - 为了追求交易速度，很多情况下交易者不得不放弃存储功能以换取更高的执行速度，这给盘后分析带来很大困难，使分析缺乏必要的数据支持。
* 交易延迟分析及优化 - 交易者缺乏测量交易延迟的标准化工具，关于延迟的统计数据往往只能凭感觉估算，在缺乏可靠的数据采集手段的情况下，优化工作变得更加艰难。

针对这两个问题，功夫在系统架构上做了特别的设计，目前系统分成以下三个模块：
* 易筋经(yijinjing) - 专为金融交易设计的时间序列内存数据库，提供纳秒级时间精度，可落地交易相关的全部数据。
* 咏春(wingchun) - 策略执行引擎，提供 C++ 及 Python (2.7) 开发接口，利用易筋经特性，咏春还提供一系列交易数据分析工具。
* 长拳(longfist) - 交易数据标准化定义及转换器，支持灵活对接不同交易柜台。

功夫在系统设计上支持任意柜台的对接（涵盖中国所有股票、期货市场），目前功夫开源版仅提供 CTP 和 XTP 柜台对接的实现。
如果需要接入更多柜台请通过 [Taurus.ai](http://taurus.ai) 官网联系我们。
开发者也可根据长拳标准自行开发新的柜台接口。

初次使用可观看我们的 [视频教程](https://www.bilibili.com/video/av16713275/)。


python策略开发请参考 [python策略开发快速入门](doc/py_strategy_guide.md)，[python策略api文档](doc/py_strategy_api.md)


C++策略开发请参考 [C++策略开发快速入门](doc/cpp_strategy_guide.md)， [C++策略api文档](doc/cpp_strategy_api.md)

常见问题请参考 [Q&A](doc/QA.md)。

想要在自己的机器上直接运行kungfu（不用docker）可以参考 [环境准备](doc/env_prep.md)

更多介绍请关注知乎专栏 [硅商冲击](https://zhuanlan.zhihu.com/silicontrader)。

License
==========

功夫采用 Apache License 2.0 发布。

Setup 编译及运行环境
=============

功夫是基于 Linux CentOS 7 开发的，需要在 CentOS 7 以上的环境中编译及运行。如果您使用 Windows 或 MacOS 或其他 Linux 发行版，建议使用虚拟机形式安装一个 CentOS 7 系统。

功夫的编译及运行依赖一系列软件包，为了简化操作，我们准备了完整的 [Docker](https://www.docker.com) 环境供大家使用。
首先需要在 CentOS 7 系统中安装及启动 Docker 服务：

```
$ sudo yum install docker
$ sudo systemctl start docker
$ sudo systemctl status docker
```

在安装好 Docker 之后，使用如下命令获取 Docker 镜像：

```
$ sudo docker pull taurusai/kungfu-devel
```

或使用我们提供的国内高速镜像：

```
$ sudo docker pull image.taurusai.com/library/kungfu-devel
```

成功获取镜像后，使用镜像启动 Docker 实例：

```
$ sudo docker run --name kungfu-devel --privileged --ulimit memlock=-1 --net=host -td taurusai/kungfu-devel /usr/sbin/init
```

推荐使用以下命令检查及维护 Docker 实例状态，更多 Docker 相关操作请移步 [Docker](https://www.docker.com) 官网查阅文档：

```
$ docker ps -a
CONTAINER ID        IMAGE                   COMMAND             CREATED             STATUS              PORTS               NAMES
024b65c6bcbf        taurusai/kungfu-devel   "/usr/sbin/init"    4 seconds ago       Up 3 seconds                            kungfu-devel

$ docker stop kungfu-devel
$ docker start kungfu-devel
```

成功启动 Docker 实例后，使用以下命令进入实例，后续操作请在 Docker 实例内完成：

```
$ sudo docker exec -it kungfu-devel bash
```

如果不使用我们提供的 Docker 编译环境，需要自行安装如下依赖包：

```
boost 1.62
rfoo 1.3.1
pid 2.1.1
log4cplus2 2.0.0_RC1
supervisor 3.1.0
```

Compile 编译
==========

获取代码：

```
$ git clone https://github.com/taurusai/kungfu
```

功夫使用 [CMake](https://cmake.org) 进行编译，CMake 会在当前路径下产生一系列中间文件，建议在单独的编译路径下进行编译：

```
$ cd kungfu
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make package
```

编译成功，会在编译路径下产生 kungfu-0.0.1-Linux.rpm，可以使用 yum 进行安装：

```
$ yum install kungfu-0.0.1-Linux.rpm
```

如需再次安装，建议先删除已安装版本：

```
$ yum erase kungfu
```

Run 启动
======

我们准备了开箱即用的 Docker 镜像，无需编译即可运行功夫，获取方式如下：

```
$ sudo docker pull image.taurusai.com/library/kungfu
$ sudo docker run --name kungfu --privileged --ulimit memlock=-1 --net=host -td taurusai/kungfu /usr/sbin/init
```

成功安装功夫 rpm 后，会自动启动后台服务，可使用 systemctl 命令查看后台状态：

```
$ sudo systemctl status kungfu
* kungfu.service - Kungfu Master Trading System Daemon
   Loaded: loaded (/opt/kungfu/master/etc/systemd/user/kungfu.service; linked; vendor preset: disabled)
   Active: active (running) since 二 2017-11-14 15:17:07 CST; 2s ago
  Process: 24778 ExecStart=/usr/bin/supervisord -c /opt/kungfu/master/etc/supervisor/supervisord.conf (code=exited, status=0/SUCCESS)
 Main PID: 24781 (supervisord)
   CGroup: /system.slice/kungfu.service
           |-24781 /usr/bin/python /usr/bin/supervisord -c /opt/kungfu/master/etc/supervisor/supervisord.conf
           |-24786 /usr/bin/python -u /opt/kungfu/master/bin/yjj server
           `-24787 /usr/bin/python -u /opt/kungfu/master/bin/yjj server
```

如果启动有问题，可以使用如下命令手动启动后台服务：

```
$ sudo systemctl start kungfu
```

相关日志存放在 /shared/kungfu/log 下。

使用 kungfuctl 命令控制后台进程：

```
$ kungfuctl
kungfu> status
md_ctp                           STOPPED   Not started
td_ctp                           STOPPED   Not started
yijinjing                        RUNNING   pid 25763, uptime 0:00:11
```

在第一次使用功夫进行交易之前，需要编辑配置文件，添加账户信息：

```
$ cp /opt/kungfu/master/etc/kungfu/kungfu.json.sample /opt/kungfu/master/etc/kungfu/kungfu.json
$ vi /opt/kungfu/master/etc/kungfu/kungfu.json
```
账户的配置信息中 FrontUri 是前置地址，如果使用的是 simnow 仿真账户可以在 simnow 官网查询, UserId 和 InvestorId 均为投资者账户，BrokerId 为券商代码，simnow 账户 BrokerId 一般为 9999。simnow投资者账户需要使用客户端登录并修改密码以后方可使用。

账户列表后的 FeeSetup 是交易费率设置，stock 为默认股票费率，future 为默认期货费率，future_exotic 为指定期货费率。其中type 为计费方式，其中 volume 是根据合约数目计费，amount 是根据合约金额计费，fee_multi 是单位费用或费用比例，ctr_multi 为单位合约标的数量，min_fee 为最小费率

正确配置功夫之后，通过 kungfuctl 命令启动行情及交易服务，检查确保服务启动正常：

```
$ kungfuctl
kungfu> start md_ctp
kungfu> start td_ctp
kungfu> status
md_ctp                           RUNNING   pid 25764, uptime 0:00:11
td_ctp                           RUNNING   pid 25765, uptime 0:00:11
yijinjing                        RUNNING   pid 25763, uptime 0:00:11
```

Run Strategy 运行策略
=================

功夫提供 C++ 及 Python (2.7) 策略开发接口，相关样例 demo 程序可在以下代码路径找到：

```
$ cd kungfu/wingchun/strategy/cpp_demo
$ cd kungfu/wingchun/strategy/py_demo
```

功夫默认安装在名为 bruce 的用户下，策略程序应以 bruce 身份运行以确保得到正确的文件读取权限（sudo -u bruce）。
以 py_demo/band_demo_strategy.py 为例，使用以下方式运行策略，需要注意 -n 参数指定的策略名称，是交易系统内对该策略的唯一标识，所有相关的交易记录都通过该标识关联：

```
$ sudo -u bruce wingchun strategy -n band_demo -p /your/path/band_demo_strategy.py
```

C++版本的策略 Demo 默认不编译，如需编译可参考如下步骤：

```
$ mkdir cpp_build
$ cd cpp_build
$ cmake ../../wingchun/strategy/cpp_demo
$ make
```

C++ Demo 编译后生成可执行文件，运行策略不是通过 wingchun 命令而是直接执行：

```
$ ./kungfu_strategy_demo
```

策略运行期间及结束后，可以使用 wingchun 命令查看策略运行状态：

```
$ wingchun help pos
$ wingchun help report
```

### 系统运行信息

系统运行期间，可使用如下命令查看系统后台信息，其中包括了系统所有进程的 PID、启动和换日时间等信息：

示例中'Pid'项表示现在'paged'内存数据服务正在运行，'MD‘行情服务正在运行，’TD‘交易服务正在运行，还有一个策略名为'demo_test’的策略正在运行。

```
$ yjj status
{'Client': {'MDEngineCTP': {'hash_code': 1788426942,
                            'is_strategy': False,
                            'is_writing': True,
                            'pid': 15493,
                            'registerTime': 1512726147926855797,
                            'rid_start': -1,
                            'trade_engine_vec': [],
                            'users': [4]},

 'Pid': {15423: ['paged'],
         15493: ['MDEngineCTP_R', 'MDEngineCTP', 'RAW_MDEngineCTP'],
         15494: ['TDEngineCTP_SR',
                 'TDEngineCTP',
                 'SEND_TDEngineCTP',
                 'RAW_TDEngineCTP'],
         16664: ['demo_test', 'demo_test_R']},
 'Task': ('running',
          10000,
          {'KfController': {'engine_ends': ['2017-12-09 16:30:00'],
                            'engine_starts': ['2017-12-09 09:15:00'],
                            'switch_day': '2017-12-09 17:00:00'},
           'PidCheck': {},
           'TempPage': {}}),
 'Time': '2017-12-08 17:52:58'
```

### 日志

系统日志存放在 /shared/kungfu/log 目录下。

|日志文件|日志内容|功能|
|:--:|:--:|:--:|
|page_engine.log|内存数据库日志|记录内存数据库及其他功能使用内存数据库的情况|
|engine_md.log|行情服务项日志|记录行情服务启动和运行情况|
|engine_trade.log|交易服务项日志|记录交易服务启动和执行情况|
|strategy/xxx.log|策略运行日志|记录策略的运行情况|
|wingchun/xxx.log|行情和交易服务的分接口详细日志|分接口记录了行情和交易服务的详细日志|

### 内存数据库数据查看与导出工具

系统提供了内存数据库查看和导出工具：

内存数据库查看工具帮助

```
$ yjj journal -h
Options:
  -h [ --help ]                         Help screen
  -n [ --name ] arg                     Journal Name
  -p [ --page ]                         Just Page Header
  -v [ --verify ]                       Verify hash code
  -k [ --keep ]                         keep listening
  -t [ --time ]                         time visualized
  -d [ --detail ]                       data details
  -c [ --current ]                      start from current
  -l [ --length ] arg (=20)             Char num of frame data to print
  -m [ --msgtype ] arg                  Message type printed, -eg: -m 10,11
  -r [ --rmsgtype ] arg                 Message type not printed, -eg: -r 10,11
  -s [ --start_time ] arg (=20000101-13:30:00)
                                        start time (%Y%m%d-%H:%M:%S)
  -e [ --end_time ] arg (=20200101-00:00:00)
                                        end time (%Y%m%d-%H:%M:%S)
```
查看内存数据库中指定信息：

```
$ yjj journal -n TD_CTP -s 20171114-14:40:00 -e 20171114-14:45:00 -d -t -m 206
  StartTime:      20171114-14:40:00
  EndTime:        20171114-14:45:00
  Folder: /shared/kungfu/journal/TD/CTP/
  ShortName:      TD_CTP
  MsgType to Print: 206
  [0] (st)1 (so)1 (na)1510641601378381049 (en)0 (vn)20171114-14:40:01 (fl)302 (dl)264 (hl)38 (hs)4109164010 (mt)206 (lf)1 (id)8000122 (er)0 (cn)9999078622
             BrokerID: (c11) 9999
               UserID: (c16) 078622
           InvestorID: (c19) 078622
         BusinessUnit: (c21)
         InstrumentID: (c31) rb1801
             OrderRef: (c21) 124
           ExchangeID: (c11) SHFE
              TradeID: (c21)       125394
           OrderSysID: (c31)       196229
        ParticipantID: (c11) 9999
             ClientID: (c21) 9999078602
                Price:   (d) 3946
               Volume:   (i) 1
           TradingDay: (c13) 20171114
            TradeTime: (c13) 14:40:00
            Direction:   (t) Sell
           OffsetFlag:   (t) Open
            HedgeFlag:   (t) Speculation
```

内存数据库导出工具帮助：

```
$ yjj dump -h
usage: journal_dumper [-h] [-n NAME] [-m MSG_TYPE] [-o OUTPUT] [-v] [-p] [-s START] [-e END]

optional arguments:
  -h, --help            show this help message and exit
  -n NAME, --name NAME  journal name
  -m MSG_TYPE, --msg_type MSG_TYPE
                        msg type to dump
  -o OUTPUT, --output OUTPUT
                        output file name
  -v, --visualize       visualize the time
  -p, --print_out       print to console while dumping
  -s START, --start START
                        start time (&Y&m&d-&H:&M:&S)
  -e END, --end END     end time (&Y&m&d-&H:&M:&S)
```

导出内存数据库中指定信息：

```
$ yjj dump -n MD_CTP -s 20171114-09:30:00 -e 20171114-16:00:00 -m 101 -o md_20171114.csv
$ head -n 3 md_20171114.csv
TradingDay(c13),InstrumentID(c31),ExchangeID(c9),ExchangeInstID(c64),LastPrice(d),PreSettlementPrice(d),PreClosePrice(d),PreOpenInterest(d),OpenPrice(d),HighestPrice(d),LowestPrice(d),Volume(i),Turnover(d),OpenInterest(d),ClosePrice(d),SettlementPrice(d),UpperLimitPrice(d),LowerLimitPrice(d),PreDelta(d),CurrDelta(d),UpdateTime(c13),UpdateMillisec(i),BidPrice1(d),BidVolume1(i),AskPrice1(d),AskVolume1(i),BidPrice2(d),BidVolume2(i),AskPrice2(d),AskVolume2(i),BidPrice3(d),BidVolume3(i),AskPrice3(d),AskVolume3(i),BidPrice4(d),BidVolume4(i),AskPrice4(d),AskVolume4(i),BidPrice5(d),BidVolume5(i),AskPrice5(d),AskVolume5(i),h_nano(l),h_msg_type(i),h_request_id(i),h_source(i),h_is_last(i),h_error_id(i),j_name(s)
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821770,32084979660.0,1697634.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:08,500,3941.0,24,3942.0,340,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640349349309138,101,-1,1,1,0,MD_CTP
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821772,32085058480.0,1697634.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:09,0,3941.0,23,3942.0,341,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640349845391063,101,-1,1,1,0,MD_CTP
```
示例中-m参数为信息分类标记，常用的信息分类如下表，全部信息分类可查看源代码 /longfist/longfist/LFConstants.h

|名称|值|含义|
|:--:|:--:|:--:|
| MSG_TYPE_LF_MD |101|tick行情数据，在journal/MD目录下相应接口目录下数据库文件中|
| MSG_TYPE_LF_BAR_MD |110|bar行情数据，在journal/MD目录下相应接口目录下数据库文件中|
| MSG_TYPE_LF_QRY_POS |201|查询持仓请求数据，在journal/strategy/相应策略数据库文件中|
| MSG_TYPE_LF_RSP_POS |202|查询持仓返回数据，在journal/TD/相应接口目录下数据库文件中|
| MSG_TYPE_LF_ORDER |204|下单请求数据，在journal/strategy/相应策略数据库文件中|
| MSG_TYPE_LF_RTN_ORDER |205|下单回报数据，在journal/TD/相应接口目录下数据库文件中|
| MSG_TYPE_LF_RTN_TRADE |206|成交回报数据，在journal/TD/相应接口目录下数据库文件中|
| MSG_TYPE_LF_ORDER_ACTION |207|撤单请求数据，在journal/strategy/相应策略数据库文件中|
| MSG_TYPE_LF_RSP_ACCOUNT |209|撤单回报数据，在journal/TD/相应接口目录下数据库文件中|

### 统计分析工具

统计分析工具帮助：

```
$ wingchun report -h
Options:
  -h [ --help ]                         Help screen
  -n [ --name ] arg                     strategy name
  -l [ --list ]                         list all sessions with index
  -o [ --order ]                        show orders
  -t [ --trade ]                        show trades
  -a [ --aggregate ]                    show aggregated latency stat
  -d [ --detail ]                       list order detail nanos
  -i [ --index ] arg (=-1)              session index
  -s [ --start_time ] arg (=20000101-13:30:00)
                                        start time (%Y%m%d-%H:%M:%S)
  -e [ --end_time ] arg (=20200101-00:00:00)
                                        end time (%Y%m%d-%H:%M:%S)
```

委托记录：

```
$ wingchun report -n band_demo -o

+-----------------------------------------------------------------------------------------------------------------+
|                                  Kungfu order latency report                                                    |
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
| order_id  | rcv_time        | TTT-b(ns)    | TTT-a(ns)    | ticker   | price    | volume   | offset       | dir |
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
|    8000001|20171204-11:01:27|          5779|         43404|    rb1801|    3967.0|         1|          Open|  Buy|
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
|    8000002|20171204-11:01:28|          5297|         51800|    rb1801|    3967.0|         1|          Open|  Buy|
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
|    8000003|20171204-11:01:29|          4965|         45227|    rb1801|    3967.0|         1|          Open|  Buy|
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
|    8000004|20171204-11:01:31|          6469|         51424|    rb1801|    3967.0|         1|          Open|  Buy|
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
|    8000005|20171204-11:01:31|          7800|         52847|    rb1801|    3967.0|         1|          Open|  Buy|
+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+
```

成交记录：

```
$ wingchun report -n band_demo -t

+-------------------------------------------------------------------------------------------+
|                                  Kungfu trade latency report                              |
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
| order_id  |idx| rcv_time        | OTR(ns)      | ticker   | t_time  | t_price  | t_volume |
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
|    8000000|  1|20171114-14:28:03|      64549194|    rb1801| 14:28:03|    3938.0|         1|
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
|    8000000|  2|20171114-14:41:32|  808582415662|    rb1801| 14:41:31|    3946.0|         1|
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
|    8000001|  1|20171114-14:28:08|      66985968|    rb1801| 14:28:07|    3939.0|         1|
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
|    8000001|  2|20171114-14:42:11|  843622609364|    rb1801| 14:42:11|    3948.0|         1|
+-----------+---+-----------------+--------------+----------+---------+----------+----------+
```

延迟统计：

```
$ wingchun report -n band_demo -a

+------------------------------------------------------------------------------------------------------+
|                                       Kungfu latency statistics                                      |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| type | description       | mean(ns)     | std(ns)      | min(ns)      | max(ns)      | valid | total |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| TTT  | (tick-to-trade)   |                                                                           |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | before API        |          4323|   3.88215e+03|          2361|         52612|   1000|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | after API         |         50880|   1.61630e+05|         35586|       5064745|   1000|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| STR  | strategy calc     |          1164|   2.43951e+03|           401|         47234|   1000|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| OTR  | (order-to-return) |                                                                           |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | rsp order insert  |             0|   0.00000e+00|             0|             0|      0|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | first rtn order   |      39174735|   1.97535e+07|      30131196|     210217560|   1000|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | first rtn trade   |             0|   0.00000e+00|             0|             0|      0|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | all rtn trade     |             0|   0.00000e+00|             0|             0|      0|   1000|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
```

注：以上样例主要展示功能性，延迟具体数字在开发机上得到，不具备参考性。关于如何得到更优的延迟数字，稍后会添加相关文档说明。

### 持仓设置工具

交易系统支持持仓导出为 csv 文件以及 csv 文件设置为持仓。

查看持仓设置工具帮助：

```
$ wingchun pos -h
Options:
  -h [ --help ]         Help screen
  -t [ --type ] arg     type ("set"/"get")
  -s [ --source ] arg   source index (eg CTP=1)
  -n [ --name ] arg     strategy name
  -o [ --output ]       output to file
  -r [ --raw ]          print raw data
  -d [ --detail ]       print user info details
  -l [ --list ]         list all files
  -j [ --json ] arg     json_file
  -c [ --csv ] arg      csv_file format:
                        ticker, net_tot, net_yd, long_tot, long_yd, short_tot,
                        short_yd                    
```

导出持仓和使用csv文件设置持仓：

```                        
$ wingchun pos -t get -n band_demo -s 1 -o -c band_demo.csv
=========== (name) band_demo (source)14 =========
nano:	1514453987908204897 (20171228-17:39:47)
ok:	Yes
-------
rb1805	(net)0,0	(long)6,6	(short)6,6	(net_c)0,0	(long_c)0,0	(short_c)0,0
-------
printed to csv file: band_demo.csv
-------
$ wingchun pos -t set -n band_demo -s 1 -c band_demo.csv
set pos from csv file: band_demo.csv
```

Version 版本
=============

* 0.0.1:
    初始化版本
* 0.0.2:
    * 修正了 PosHandler 的一个 update 情况的潜在风险
    * 修正没有 close 的 file 句柄
    * 修正了 memcpy 的潜在越界问题
    * 编译选项优化为 O3
* 0.0.3:
    * 增强 wingchun report 中的延迟统计工具，新增调用API前的系统内耗时 (TTT before API)
* 0.0.4:
    * 增加 FeeHandler 模块，增加策略中的 Pnl 实时计算支持
* 0.0.5:
    * 增加对股票交易柜台 xtp 的支持
    * 在系统 docker 中增加了 numa（xtp 的依赖），不希望更新 docker 的用户可以通过 yum install numactl 来手动安装

Contribute 开发
=============

开发文档即将上线，请关注 [Taurus.ai](http://taurus.ai) 官网。
QQ 交流群 312745666，入群问题答案：taurus.ai