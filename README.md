Intro 简介
========

功夫是 [Taurus.ai](http://taurus.ai) 团队专为量化交易者设计的开源交易执行系统。功夫想要解决以下问题：
* 低延迟交易 - 量化交易者对系统内响应速度有极高要求，功夫提供微秒级别的系统响应，支持带纳秒级时间戳的交易数据实时存储和盘后分析。
* 开放的策略编写方式 - 功夫支持 Python 2/3 及 C++ 形式的策略编写，策略师可以不受限的自由使用第三方计算库，放飞创意。
* 友好的使用方式 - 告别 Linux shell 小黑屋，功夫提供图形化操作界面，简化策略运维流程。而进阶用户仍然具备通过底层 API 以无界面形式使用系统的能力。
* 跨平台运行 - 三大主流平台（Windows、MacOSX、Linux）皆可编译运行。
* 灵活的扩展接口 - 功夫提供几种不同的数据交互接口（易筋经、SQLite、nanomsg），支持用户自行开发各种功能模块。

功夫系统架构如下：
* 后台核心（C++）
  * 易筋经（yijinjing） - 专为金融交易设计的超低延迟时间序列内存数据库，提供纳秒级时间精度，可落地交易相关的全部数据。
  * 咏春（wingchun） - 策略执行引擎，提供策略开发接口，利用易筋经特性，咏春还提供一系列交易数据分析工具。
* 中台交互（C++/Python/nodejs）
  * [SQLite](https://www.sqlite.org/index.html) - 功夫使用内嵌式数据库 SQLite 存储配置信息及中间数据
  * [nanomsg](https://nanomsg.org) - 功夫使用 nanomsg 作为前后台通信机制，系统内对延迟不敏感的指令（例如手动下单等）可通过 nanomsg 信道传达。
* 前端UI（nodejs）
  * [Electron](https://electronjs.org) - 跨平台的桌面应用开发框架
  * [Vue.js](https://vuejs.org) - UI开发框架
* 打包机制
  * [pyinstaller](https://pyinstaller.readthedocs.io/en/stable/) - 封装完整的 Python 环境及所有二进制依赖，使得功夫的安装部署绿色化，无需依赖其他软件。
  * [pipenv](https://pipenv.readthedocs.io) - Python 依赖管理工具，开发过程无需额外安装 Python 包。
  * [pm2](https://pm2.io) - 基于 nodejs 的进程管理工具
  * [electron-builder](https://www.electron.build) - 借助 Electron 技术，功夫可以提供本地化应用程序的安装使用体验。

功夫在系统设计上支持任意柜台的对接（涵盖中国所有股票、期货市场），目前功夫开源版仅提供 CTP 和 XTP 柜台对接的实现。
如果需要接入更多柜台请通过 [Taurus.ai](http://taurus.ai) 官网联系我们。
开发者也可根据长拳标准自行开发新的柜台接口。

初次使用请参考 [功夫交易系统用户手册](https://app.gitbook.com/@taurusai/s/kungfu-userdoc/)

更多介绍请关注知乎专栏 [硅商冲击](https://zhuanlan.zhihu.com/silicontrader)。

License
==========

功夫采用 Apache License 2.0 发布。

Setup 编译及运行环境
=============

功夫的编译依赖以下工具：
git
Node.js (>=8 <11)
yarn
Python 2
pipenv
cmake (>3.12)

功夫的编译需要系统默认（$PATH指向）的 Python 版本为 2。如果系统中也存在 Python 3，则可以通过下述 pipenv 的方式使得最终输出的 app 使用 Python 3，但仍然需要默认的 Python 是 2。

功夫编译依赖 [Node.js](https://nodejs.org)，建议预先进行如下设置加速依赖包的下载：
```
$ npm config set registry https://registry.npm.taobao.org
$ npm config set electron_mirror https://npm.taobao.org/mirrors/electron/
$ npm config set PUPPETEER_DOWNLOAD_HOST https://npm.taobao.org/mirrors
$ npm config set puppeteer_download_host https://npm.taobao.org/mirrors
$ npm config set sass-binary-site http://npm.taobao.org/mirrors/node-sass
```

#### MacOSX

```
$ brew install git cmake node@10
$ npm install -g yarn electron-builder
$ pip install pipenv
```

#### Windows

开发组在 Visual Studio 2017 15.9.11 环境下进行工作，安装时需要勾选 VC140（Visual Studio 2015) toolset。

下载并安装 [git](https://git-scm.com/download/win)，[Python](https://www.python.org/downloads/windows/)，[CMake](https://cmake.org/install/)，[Node.js LTS 10.15.3](https://nodejs.org/en/download/) 并添加相应路径至 %PATH% 环境变量。

Windows需要额外安装 [Boost 1.64.0](https://sourceforge.net/projects/boost/files/boost-binaries/1.64.0/)，下载并安装 boost_1_64_0-msvc-14.1-64.exe。

```
C:\> npm instal -g yarn electron-builder
C:\> pip install pipenv
```

#### Linux

```
$ # install git cmake node.js
$ node-v10.15.3-linux-x64/bin/npm install -g yarn electron-builder
$ pip install pipenv
```

Compile 编译
==========

#### 常规操作

获取代码并编译：
```
$ git clone https://github.com/taurusai/kungfu
$ cd kungfu
$ yarn install
$ yarn run build
```

编译结果输出在 build 目录下，例如在 MacOSX 系统上，最终的可执行文件输出在 build/mac/Kungfu.Trader.app。

遇到编译问题需要完整的重新编译时，执行以下命令清理临时文件：
```
$ yarn run clean
```

#### 选择编译模式

功夫默认编译为 Release 模式（-D[CMAKE_BUILD_TYPE](https://cmake.org/cmake/help/v3.12/variable/CMAKE_BUILD_TYPE.html)="Release")，如果希望以 Debug 模式编译，需要执行以下命令：
```
$ yarn config set kungfu:cmakejsopt "debug"
```

执行以下命令恢复 Release 模式：
```
$ yarn config set kungfu:cmakejsopt
```

更多可选设置请参考 [CMake.js Options](https://www.npmjs.com/package/cmake-js)。

切换编译模式后，需要执行以下命令重新生成配置文件：
```
$ yarn run config
```


#### 选择 Python 版本

功夫支持 Python 2 及 Python 3，在系统预装了相应版本的情况下，编译时可以自行选择所需的 Python 版本。
执行以下命令选择 Python 3：
```
$ yarn config set kungfu:pyver three
```

执行以下命令选择 Python 2：
```
$ yarn config set kungfu:pyver two
```

切换 Python 版本后，需要执行以下命令重新生成配置文件：
```
$ yarn run config
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
$ yarn run clean
```
需要注意 node_modules 目录为 npm 产生的包目录，一般情况下无需清除，如有特殊需要可手动删除。

另外，编译过程中会在系统的以下路径产生输出：
```
$HOME/.cmake-js                     # cmake.js 存储的 C++ 依赖包
$HOME/.virtualenvs                  # pipenv(windows) 存储的 Python 依赖
$HOME/.local/share/virtualenvs      # pipenv(unix) 存储的 Python 依赖
```
如果需要清理这些文件，都需要手动删除。

Version 版本
=============

* 2.0.0:
    * 跨平台支持
    * 支持 Python 2/3
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
    初始化版本

Contribute 开发
=============

开发文档即将上线，请关注 [Taurus.ai](http://taurus.ai) 官网。
QQ 交流群 312745666，入群问题答案：taurus.ai