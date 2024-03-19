# Intro 简介

[功夫核心库](https://libkungfu.cc) 是专为量化交易者设计的开源交易执行系统。功夫想要解决以下问题：
* 低延迟交易 - 量化交易者对系统内响应速度有极高要求，功夫提供微秒级别的系统响应，支持带纳秒级时间戳的交易数据实时存储和盘后分析。
* 开放的策略编写方式 - 功夫支持 Python 3 及 C++ 形式的策略编写，策略师可以不受限的自由使用第三方计算库，放飞创意。
* 友好的使用方式 - 告别 Linux shell 小黑屋，功夫提供图形化操作界面，简化策略运维流程。而进阶用户仍然具备通过底层 API 以无界面形式使用系统的能力。
* 跨平台运行 - 三大主流平台（Windows、MacOSX、Linux）皆可编译运行。

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

功夫在系统设计上支持任意柜台的对接（涵盖中国所有股票、期货市场），功夫开源版提供 [XTP](https://xtp.zts.com.cn/) 柜台对接的参考实现。
如果需要接入更多柜台请至 [功夫量化](https://www.kungfu-trader.com) 下载标准版或联系我们。

初次使用请参考 [功夫核心库文档](https://docs.libkungfu.cc)。

# License

Apache License 2.0

# Setup 编译及运行环境

功夫的编译依赖以下工具：

* 支持 C++20 的编译器
* [cmake](https://cmake.org/) (>=3.15)
* [Node.js](https://nodejs.org/) (^14.x)
* [yarn](https://classic.yarnpkg.com/) (^1.x)
* [Python 3](https://www.python.org/) (~3.9)
* [pipenv](https://pipenv.pypa.io/) (>=2023.9.1)

开始编译前，请先确保安装以上工具，且正确设置 PATH 环境变量。

# Compile 编译

#### 常规操作

获取代码并编译(必须用git方式获取代码，功夫编译需要获取git仓库的版本信息）：
```
## git clone kungfu repo
$ cd kungfu
$ yarn install --frozen-lockfile
$ yarn build
$ yarn package
```

编译结果输出在 artifact/build 目录下。

遇到编译问题需要完整的重新编译时，执行以下命令：
```
$ yarn rebuild
$ yarn package
```

#### 编译过程产生的临时文件

编译过程会在代码所在目录下生成如下临时文件：
```
node_modules
**/node_modules
**/build
**/dist
```

通常情况下可通过执行如下命令对 build 和 dist 进行清理：
```
$ yarn clean
```
需要注意 node_modules 目录为 yarn 产生的包目录，一般情况下无需清除，如有特殊需要可手动删除。

另外，编译过程中会在系统的以下路径产生输出：
```
$HOME/.conan                        # [conan](https://conan.io/center/) 的配置信息以及其存储的 C++ 依赖包
$HOME/.cmake-js                     # [cmake.js](https://www.npmjs.com/package/cmake-js) 存储的 C++ 依赖包
$HOME/.virtualenvs                  # pipenv(windows) 存储的 Python 依赖
$HOME/.local/share/virtualenvs      # pipenv(unix) 存储的 Python 依赖
```
如果需要清理这些文件，都需要手动删除。

# Help 帮助信息

更多信息请访问网站 [功夫核心库](https://libkungfu.cc) 及 [功夫量化](https://www.kungfu-trader.com)。

微信公众号：功夫量化
