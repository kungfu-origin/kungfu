
系统相关问题
===========
### Q: 功夫开源系统使用的什么许可模式

A: MIT 许可，可自由使用

#### Q: 功夫交易系统可以做回测吗？

A:目前不包括回测

#### Q: 使用CTP连接延时抖动很大，功夫交易系统达到高速是否有意义?

A:首先功夫提供的是一个开源的框架，使用这个系统你可以准确的得到系统内部的延时，这对标准化评估策略是有意义的。其次功夫交易系统标准版也提供了更多其他更为稳定和极速的接口。再次，不考虑速度优势，功夫交易系统也是一个稳定好用的交易系统。

#### Q: 预期功夫开源交易系统官方还会添加哪些功能？

A: 接入股票交易，持仓盈亏统计，风控等。



#### Q: 如何开始使用功夫开源交易系统?

A:可以按照[功夫交易系统介绍](../READEME.md)的操作流程完成系统的获取、安装与策略的运行，熟悉系统，然后参考帮助文档实现自己的策略，如果有不理解的地方欢迎在github上提出或在讨论群(群号:312745666)中讨论。

#### Q: 为什么我测试得到的系统延时较大?

A:要达到极致延时体验，需要在性能较好的物理服务器上，采用核隔离和绑核操作，保证交易系统资源不被占用的情况下达到的。

#### Q: 为什么系统晚上行情交易连接自动会中断

A:系统默认的交易时间是 09:15-16:30,如果你需要交易夜盘，需要在 /opt/kungfu/master/bin/yjj 的195行以后添加如下代码，重启kungfu，即可将夜盘交易时间添加到交易时段中。

```
        task_control.set_switch_day_time('17:00:00')
        task_control.add_engine_start_time('09:15:00')
        task_control.add_engine_end_time('16:30:00')
        task_control.add_engine_start_time('20:30:00')
        task_control.add_engine_end_time('02:40:00')
```


策略相关问题
==========

#### Q: 如何开始自己的第一个策略？

A:建议先通过结合帮助和 py_demo 目录下的 demo，熟悉策略框架，然后在 basic_usage.py 的回调函数中调试打印收到的数据，熟悉数据结构。然后调用功能函数，实现订阅，查持仓，下单，添加回调函数等函数使用。在此基础上，实现自己的交易算法。

#### Q: 为什么py_demo里面的策略运行了但是什么输出也没有？

A：一种可能是系统的内存数据库服务并没有启动起来或者行情和交易没有连接，可以使用 systemctl status kungfu 查看交易系统是否启动，或者使用 kungfuctl 查看其中 yjj 状态是否正常，如果状态不正常可以重启 kungfu，重启kungfu以后需要启动 md 和 td 连接，然后再启动交易。

另一种可能是策略没有初始化持仓。py_demo 里面的策略示例，band_demo_strategy.py 是一个完整的策略，其他策略都是单项或者简单功能展示，策略如果首次运行，需要设置策略的持仓。py_demo 文件夹中 initial_pos_test.py 正是展示的初始化策略持仓的过程。所以执行其他策略示例前可以先执行 initial_pos_test.py， 例如按如下顺序执行。

```
$ wingchun strategy -n test -p initial_pos_test.py
$ wingchun strategy -n test -p order_cancel_test.py
```

#### Q: 策略名和策略文件名之间有什么关系吗？

A：python策略中，策略名为运行策略是 -n 参数后的信息，-p 参数后为策略脚本，二者之间是完全独立的关系，-n后的策略名是策略在系统中的唯一标识。如下示例中，“test” 是策略的名称，两次执行虽然使用的脚本文件不同，但是对于系统来说他们都是同一个策略。

```
$ wingchun strategy -n test -p initial_pos_test.py
$ wingchun strategy -n test -p order_cancel_test.py
```

#### Q: 为什么策略运行会报"RuntimeError: cannot register client: strategy_name "错误

A:这是因为已经有一个同名策略在运行了。可以参考下面的如何关闭策略，先关闭同名策略，然后继续启动，或者换一个名字。
功夫交易系统中启动策略是 -n 参数后面的名字是策略的唯一标识，同一时刻，不能有两个同名的策略。

#### Q: 如果只需要订阅应该如何实现？

A:可以写一个只包含订阅功能的策略实现订阅功能，可以将订阅列表直接添加在代码中，也可以从外部数据导入。数据订阅后，只要 wingchunctl 中的 md 不关闭，系统每天会自动订阅之前订阅过的数据。比如：

```
#file sub.py
#run: wingchun strategy -n SUB -p sub.py

def initialize(context):
    context.add_md(source=SOURCE.CTP)
    context.subscribe(["rb1801","rb1803","IC1801"], source=SOURCE.CTP)
```

#### Q: 如何判断策略状态正常？

A:连接了行情和交易的策略启动以后应收到如下两行数据表明策略启动正常，如果没有第二行信息，说明策略持仓没有被初始化或者没有成功连接到交易接口。
```
[17-12-27 14:19:43.621] [INFO ] [standard_test] [start] data started
[17-12-27 14:19:43.642] [INFO ] [standard_test] [POS] strategy position:{"Cost":{"rb1805":[0.0,0.0,0.0,0.0,0.0,0.0]},"FeeSetup":{"future":{"ctr_multi":200,"fee_multi":[2.3e-05,0.00069,2.3e-05],"min_fee":0.0,"type":"amount"},"future_exotic":{"rb":{"ctr_multi":10,"fee_multi":0.0001,"min_fee":0.0,"type":"amount"}},"stock":{"ctr_multi":1,"fee_multi":0.0003,"min_fee":0.0,"type":"amount"}},"Pos":{"rb1805":[0,0,6,6,6,6]},"Source":14,"ok":true}
```
如果需要看到自己的策略持续运行的状态，可以将过程信息使用 log 工具输出出来。
如果策略是后台运行的，可以在 /shared/kungfu/log/strategy/ 中查看对应的 log 信息，通过 log 判断策略执行情况。

#### Q: 如果有多个账户，如何设置保证策略是在某账户下交易的？

A:在多个账户的情况下，如果不做任何设置，策略默认执行在默认账户下（在 kungfu.json 中可以看到账户中的 "is_default": true 项），如果要将策略添加在某个账户下，需要将策略名（-n 参数后的名字）添加到client中。
如下例子中策略名为BlDemo1, demo_21, demo_22的策略会通过该账户交易。
```
    {
      "info":
      {
        "UserId": "105507",
        "InvestorId": "105507",
        "BrokerId": "9999",
        "Password": "Ctp123456"
      },
      "clients": ["BlDemo1","demo_21","demo_22"],
      "is_default": false
    }
```

#### Q: 如何关闭策略？

A：如果在 shell 中前台运行策略，可以使用 Ctrl+c 关闭。如果策略是后台运行，或者 shell 连接中断了，可以使用yjj status ,查看 pid 中策略对应的 pid 号，使用 kill命令杀掉对应的进程（可能出现权限不足的情况，杀掉以后需检查确认）。

#### Q: 这么好的文档，是谁写的？我想问问题怎么办

A：目前是由 taurus.ai 的风流倜傥的 qianyong.liu 主要贡献的，changhao.jiang 亦有参与。欢迎参与到文档开发中，有意向请联系 qianyong.liu@taurus.ai
更多问题可在 github 上提 issue，或者在功夫开源交易系统 QQ 讨论群提出（群号:312745666），或者直接联系 qianyong.liu.