# 功夫 C++ 策略示例

## 编译方法
执行如下命令：
```
cd examples/strategy/cpp
yarn install
yarn build
```
成功后会在 build/Release 下生成如 cpp_demo.cpython-37m-darwin.so 样的动态链接库

## 运行方法（后台）
同运行 python 策略一样，通过 kfc strategy 命令完成，只需将 -p 参数指向生成的动态链接库的路径：
```
kfc strategy -n demo -p $KF_CODE/examples/strategy/demo/cpp_demo.cpython-37m-darwin.so
```
