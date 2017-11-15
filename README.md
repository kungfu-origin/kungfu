Intro 简介
========

功夫是 [Taurus.ai](http://taurus.ai) 团队专为量化交易者设计的低延迟交易执行系统。功夫想要解决两个核心问题：
* 交易数据实时存储 - 为了追求交易速度，很多情况下交易者不得不放弃存储功能以换取更高的执行速度，这给盘后分析带来很大困难，使分析缺乏必要的数据支持。
* 交易延迟分析及优化 - 交易者缺乏测量交易延迟的标准化工具，关于延迟的统计数据往往只能凭感觉估算，在缺乏可靠的数据采集手段的情况下，优化工作变得更加艰难。

针对这两个问题，功夫在系统架构上做了特别的设计，目前系统分成以下三个模块：
* 易筋经(yijinjing) - 专为金融交易设计的时间序列内存数据库，提供纳秒级时间精度，可落地交易相关的全部数据。
* 咏春(wingchun) - 策略执行引擎，提供 C++ 及 Python (2.7) 开发接口，利用易筋经特性，咏春还提供一系列交易数据分析工具。
* 长拳(longfist) - 交易数据标准化定义及转换器，支持灵活对接不同交易柜台。

功夫在系统设计上支持任意柜台的对接（涵盖中国所有股票、期货市场），目前功夫开源版仅提供 CTP 柜台对接的实现。
如果需要接入更多柜台请通过 [Taurus.ai](http://taurus.ai) 官网联系我们。
开发者也可根据长拳标准自行开发新的柜台接口。

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

Run 启动
======

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
kungfu/wingchun/strategy/cpp_demo
kungfu/wingchun/strategy/py_demo
```

功夫默认安装在名为 bruce 的用户下，策略程序应以 bruce 身份运行以确保得到正确的文件读取权限（sudo -u bruce）。
以 py_demo/band_demo_strategy.py 为例，使用以下方式运行策略，需要注意 -n 参数指定的策略名称是系统内对该策略的唯一标识，所有相关的交易记录都通过该标识关联：

```
$ sudo -u bruce wingchun strategy -n band_demo -p /your/path/band_demo_strategy.py
```

策略运行期间及结束后，可以使用 wingchun 命令查看策略运行状态：

```
$ wingchun help pos
$ wingchun help report
```

### 内存数据库 yjj journal 数据导出展示

使用 yjj 命令查看及导出易筋经内存数据库中的内容：

```
$ yjj journal -h
$ yjj journal -f /shared/kungfu/journal/TD/CTP/ -n TD_CTP  -s 20171114-14:40:00 -e 20171114-14:45:00 -d -t -m 206
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

```
$ yjj dump -f /shared/kungfu/journal/MD/CTP/ -n MD_CTP -s 20171114-09:30:00 -e 20171114-16:00:00 -m 101 -o md_20171114.csv
$ head -n 5 md_20171114.csv
TradingDay(c13),InstrumentID(c31),ExchangeID(c9),ExchangeInstID(c64),LastPrice(d),PreSettlementPrice(d),PreClosePrice(d),PreOpenInterest(d),OpenPrice(d),HighestPrice(d),LowestPrice(d),Volume(i),Turnover(d),OpenInterest(d),ClosePrice(d),SettlementPrice(d),UpperLimitPrice(d),LowerLimitPrice(d),PreDelta(d),CurrDelta(d),UpdateTime(c13),UpdateMillisec(i),BidPrice1(d),BidVolume1(i),AskPrice1(d),AskVolume1(i),BidPrice2(d),BidVolume2(i),AskPrice2(d),AskVolume2(i),BidPrice3(d),BidVolume3(i),AskPrice3(d),AskVolume3(i),BidPrice4(d),BidVolume4(i),AskPrice4(d),AskVolume4(i),BidPrice5(d),BidVolume5(i),AskPrice5(d),AskVolume5(i),h_nano(l),h_msg_type(i),h_request_id(i),h_source(i),h_is_last(i),h_error_id(i),j_name(s)
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821770,32084979660.0,1697634.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:08,500,3941.0,24,3942.0,340,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640349349309138,101,-1,1,1,0,MD_CTP
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821772,32085058480.0,1697634.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:09,0,3941.0,23,3942.0,341,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640349845391063,101,-1,1,1,0,MD_CTP
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821814,32086713700.0,1697614.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:09,500,3941.0,4,3942.0,341,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640350560025741,101,-1,1,1,0,MD_CTP
20171114,rb1801,,,3941.0,3860.0,3894.0,1794552.0,3900.0,3950.0,3873.0,821824,32087107800.0,1697616.0,1.79769313486e+308,1.79769313486e+308,4130.0,3589.0,1.79769313486e+308,1.79769313486e+308,14:19:10,0,3940.0,22,3942.0,343,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1.79769313486e+308,0,1510640351260245676,101,-1,1,1,0,MD_CTP
```

### 统计分析工具 wingchun report 样本输出展示

委托记录：

```
$ wingchun report -n band_demo -o

+--------------------------------------------------------------------------------------------------+
|                                  Kungfu order latency report                                     |
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
| order_id  | rcv_time        | TTT(ns)      | ticker   | price    | volume   | offset       | dir |
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
|    8000000|20171114-14:28:03|        325659|    rb1801|    3589.0|         1|          Open| Sell|
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
|    8000001|20171114-14:28:08|        166371|    rb1801|    4130.0|         1|    CloseToday|  Buy|
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
|    8000002|20171114-14:28:08|        102215|    rb1801|    3589.0|         1|          Open| Sell|
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
|    8000003|20171114-14:28:17|        170853|    rb1801|    4130.0|         1|    CloseToday|  Buy|
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
|    8000004|20171114-14:28:26|        184837|    rb1801|    3589.0|         1|          Open| Sell|
+-----------+-----------------+--------------+----------+----------+----------+--------------+-----+
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
| type | description       | max(ns)      | min(ns)      | mean(ns)     | std(ns)      | valid | total |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| TTT  | tick-to-trade     |        458974|         72643|        134050|   4.79453e+04|    259|    259|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| STR  | strategy calc     |         91147|         16910|         27060|   9.12920e+03|    259|    259|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
| OTR  | (order-to-return) |                                                                           |
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | rsp order insert  |             0|             0|             0|   0.00000e+00|      0|    259|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | first rtn order   |     295907970|      31956202|      40422306|   3.33506e+07|    259|    259|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | first rtn trade   |     358749574|      35815971|      82466795|   6.17791e+07|    259|    259|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
|      | all rtn trade     |     358749574|      35815971|    9758236640|          -nan|    262|    262|
+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+
```

Contribute 开发
=============

开发文档即将上线，请关注 [Taurus.ai](http://taurus.ai) 官网。
