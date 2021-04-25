# Intro 简介

[功夫](https://www.kungfu-trader.com) 是专为量化交易者设计的开源交易执行系统。功夫想要解决以下问题：
* 低延迟交易 - 量化交易者对系统内响应速度有极高要求，功夫提供微秒级别的系统响应，支持带纳秒级时间戳的交易数据实时存储和盘后分析。
* 开放的策略编写方式 - 功夫支持 Python 3 及 C++ 形式的策略编写，策略师可以不受限的自由使用第三方计算库，放飞创意。
* 友好的使用方式 - 告别 Linux shell 小黑屋，功夫提供图形化操作界面，简化策略运维流程。而进阶用户仍然具备通过底层 API 以无界面形式使用系统的能力。
* 跨平台运行 - 三大主流平台（Windows、MacOSX、Linux）皆可编译运行。
* 灵活的扩展接口 - 功夫提供几种不同的数据交互接口（易筋经、SQLite、nanomsg），支持用户自行开发各种功能模块。

功夫系统架构如下：
* 后台核心（C++）
  * 长拳（longfist） - 金融交易相关的数据格式定义，提供涵盖 c++/python/javascript/sqlite 的序列化支持。
  * 易筋经（yijinjing） - 专为金融交易设计的超低延迟时间序列内存数据库，提供纳秒级时间精度，可落地交易相关的全部数据。
  * 咏春（wingchun） - 策略执行引擎，提供策略开发接口，实时维护策略账目及持仓情况。
* 策略接口（C++/Python）
  * [RxCpp](https://github.com/ReactiveX/RxCpp) - 响应式事件处理框架，可对丰富数据类型的金融交易数据进行灵活处理。
  * numpy/pandas - 自带的 Python 运行环境原生提供 numpy/pandas 等工具供策略使用。
* 前端UI（Node.js）
  * [Electron](https://electronjs.org) - 跨平台的桌面应用开发框架
  * [Vue.js](https://vuejs.org) - UI开发框架

功夫在系统设计上支持任意柜台的对接（涵盖中国所有股票、期货市场），目前功夫开源版仅提供 CTP 和 XTP 柜台对接的实现。
如果需要接入更多柜台请通过 [功夫官网](https://www.kungfu-trader.com) 联系我们。
开发者也可根据代码示例自行开发新的柜台接口。

初次使用请参考 [使用手册](https://www.kungfu-trader.com/manual/) 及 [API文档](https://www.kungfu-trader.com/api-doc/)。

更多介绍请关注知乎专栏 [硅商冲击](https://zhuanlan.zhihu.com/silicontrader)。

# License

功夫采用 Apache License 2.0 发布。

# Setup 编译及运行环境

功夫的编译依赖以下工具：
支持 C++17 的编译器
Node.js (>=10.16.0 <11.0.0)
yarn
Python 3
pipenv
cmake (>3.15)

功夫编译依赖 [Node.js](https://nodejs.org)，建议预先进行如下设置加速依赖包的下载：
```
npm config set registry https://registry.npm.taobao.org
npm config set puppeteer_download_host https://npm.taobao.org/mirrors
npm config set electron_mirror https://npm.taobao.org/mirrors/electron/
npm config set sass-binary-site https://npm.taobao.org/mirrors/node-sass
npm config set npm_config_disturl=https://npm.taobao.org/mirrors/atom-shell
```

:electron-builder-binaries 这个依赖因为众所周知的原因可能会出现下载失败，可以通过 https://my.oschina.net/u/1240907/blog/1926061 这篇文章中的方法解决

#### MacOSX

```
brew install git cmake node@10
npm install -g yarn electron-builder
pip install pipenv
```

安装过程中如果遇到 Python 崩溃的问题，可能是由于 libssl 版本不兼容的问题，请尝试参照以下[方法](https://forums.developer.apple.com/thread/119429)：

```
brew update & brew upgrade & brew install openssl
cd /usr/local/lib
sudo ln -s /usr/local/Cellar/openssl/1.0.2s/lib/libssl.1.0.0.dylib libssl.dylib
sudo ln -s /usr/local/Cellar/openssl/1.0.2s/lib/libcrypto.1.0.0.dylib libcrypto.dylib
```

#### Windows

开发组在 Visual Studio 2019 16.5.0 环境下进行工作。

下载并安装 [git](https://git-scm.com/download/win)，[Python 3](https://www.python.org/downloads/windows/)，[CMake](https://cmake.org/install/)，[Node.js LTS 10.16.0](https://nodejs.org/en/download/) 并添加相应路径至 %PATH% 环境变量。

```
C:\> npm install -g yarn electron-builder
C:\> pip install pipenv
```

#### Linux

确保编译器支持 C++ 17，例如对于 CentOS 7，升级 gcc 到 9.x：
```
yum install -y centos-release-scl
yum-config-manager --enable centos-sclo-rh-testing
yum-config-manager --enable centos-sclo-sclo-testing
yum install -y devtoolset-9
echo "source /opt/rh/devtoolset-9/enable" >> /etc/profile
source /etc/profile
```

安装 cmake3 及 nodejs
```
$ # install rpm-build cmake3 node.js pipenv
$ yum install -y rpm-build centos-release-scl rh-nodejs10
$ pip3 install pipenv --index http://mirrors.aliyun.com/pypi/simple --trusted-host mirrors.aliyun.com
```

# Compile 编译

#### 常规操作

获取代码并编译：
```
$ git clone https://github.com/taurusai/kungfu
$ cd kungfu
$ yarn
$ yarn build
```

编译结果输出在 app/build 目录下，例如在 MacOSX 系统上，最终的可执行文件输出在 app/build/mac/Kungfu.Trader.app。

遇到编译问题需要完整的重新编译时，执行以下命令清理临时文件：
```
$ yarn clean
```

#### 选择编译模式

功夫默认编译为 Release 模式（-D[CMAKE_BUILD_TYPE](https://cmake.org/cmake/help/v3.12/variable/CMAKE_BUILD_TYPE.html)="Release")，如果希望以 Debug 模式编译，需要执行以下命令：
```
$ npm config set kungfu-core:buildtype "Debug"
```

执行以下命令恢复 Release 模式：
```
$ npm config set kungfu-core:buildtype "Release"
```

切换编译模式后，需要执行以下命令重新生成配置文件：
```
$ yarn clean
$ yarn build
```


#### 编译过程产生的临时文件

编译过程会在代码所在目录下生成如下临时文件：
```
node_modules
build
dist
```
通常情况下可通过执行如下命令对 build 和 dist 进行清理：
```
$ yarn clean
```
需要注意 node_modules 目录为 npm 产生的包目录，一般情况下无需清除，如有特殊需要可手动删除。

另外，编译过程中会在系统的以下路径产生输出：
```
$HOME/.conan                        # conan 存储的 C++ 依赖包
$HOME/.cmake-js                     # cmake.js 存储的 C++ 依赖包
$HOME/.virtualenvs                  # pipenv(windows) 存储的 Python 依赖
$HOME/.local/share/virtualenvs      # pipenv(unix) 存储的 Python 依赖
```
如果需要清理这些文件，都需要手动删除。


# Version 版本

* 2.3.4:
    * 新增行情模块
    * 新增策略追踪功能
    * 优化启停任务体验，增加td/md进程断开自动重启
    * 解决底层bug
* 2.3.3:
    * 优化手动下单联想输入
* 2.3.2:
    * 新增交易任务模块（pro）
    * 解决bug
* 2.3.1:
    * 解决底层bug
* 2.3.0:
    * 增加了对过去数据的归档功能 
    * 对手动下单功能进行优化
    * 增加标的维度信息查看
    * 解决底层bug
* 2.2.8:
    * md/td列表，表头，连接 → 进程
    * pos列表双击打开下单面板 → 单击打开
    * 当为股票账户时，下单界面新增 “总金额” 以及按数量/按金额选择
    * 当选择“按金额”时，输入“总金额”，在右侧会显示 “下单手数” = Math.floor(totalPrice / limit_price)
    * 仅当 股票-限价，会有以上选择，期货跟市价单跟之前一致
    * 下单界面新增“套保投机”
    * 期货联想ticker 新增套利组合
* 2.2.7:
    * 将之前“开启全部”更换为“保持开启”
    * 点击保持开启按钮后，会每隔2s运行一次如下逻辑：
    * 将当前未启动的进程启动，启动间隔为1s，都运行结束后，隔2s再运行下一个逻辑loop
    * 修复了下单窗口打开过慢的问题
    * 修复了策略编辑窗口打开过慢的问题
    * 保持开启更换为开启全部
    * 每次点击开启全部后，会尝试四次
    * 全部开启后，开启全部按钮变为关闭全部
* 2.2.6:
    * 增加启动全部td/md，中间间隔1s
* 2.2.5:
    * Td/md增加一键开启
    * 调仓不再需要二次确认
    * 确认下单不会受frozen持仓影响，应直接报错
    * 对于orderList，双击撤单，单击调仓，右键看order信息
    * 打开下单窗口过慢的问题
* 2.2.4:
    * 手动下单面板
    * 总：股票期货，下单输入项包括：ticker，side，offset，市价/限价（默认），限定价格，按数量/金额（买入默认按金额，卖出默认按数量）
    * 直接下单
    * ticker输入带联想，根据账户券商/期货公司识别大小写ticker
    * 从position列表进入下单
    * 识别当前pos的ticker，价格（市价/成交价？），side（相反），offset，数量/金额
* 2.2.3:
    * bug修复
* 2.2.2:
    * bug修复
* 2.2.1:
    * 支持查看历史委托及成交
    * 支持查看委托详情
    * 支持调整页面布局
* 2.2.0:
    * 优化前后端通信方式
    * 添加实时延迟统计功能
    * 支持极速模式运行
    * 调整账目持仓统计方式
    * 添加文本操作界面（TUI）功能 kungfu-cli
* 2.1.0:
    * 添加策略获取账户持仓接口
    * 区分股票与期货持仓数据结构
    * 添加bar订阅与回调
* 2.0.0:
    * 跨平台支持
    * 支持 Python 3
    * 提供基于 Electron 的图形化操作界面
* 1.0.0:
    * 以 Docker/rpm 方式运行的最后稳定版本
* 0.0.5:
    * 增加对股票交易柜台 xtp 的支持
    * 在系统 docker 中增加了 numa（xtp 的依赖），不希望更新 docker 的用户可以通过 yum install numactl 来手动安装
* 0.0.4:
    * 增加 FeeHandler 模块，增加策略中的 Pnl 实时计算支持
* 0.0.3:
    * 增强 wingchun report 中的延迟统计工具，新增调用API前的系统内耗时 (TTT before API)
* 0.0.2:
    * 修正了 PosHandler 的一个 update 情况的潜在风险
    * 修正没有 close 的 file 句柄
    * 修正了 memcpy 的潜在越界问题
    * 编译选项优化为 O3
* 0.0.1:
    * 初始化版本

# Contribute 开发

使用手册及API文档请访问 [功夫官网](https://www.kungfu-trader.com)。
QQ 交流群 312745666，入群问题答案：kungfu.trader