# libfs C++ 内存与模块示例

当前模板使用 libfs 0.1.0 静态库，提供 Android arm64-v8a、x86_64 两架构，最低 API 24。已经移除 libSea3 和旧 smemory 全局接口，不能用替换文件名的方式沿用旧调用。

## 构建与运行

需要 Android NDK，使用 C++17 与 c++_static：

```sh
export ANDROID_NDK=/你的/android-ndk
bash 一键构建脚本.sh
# 将对应架构的 libs/arm64-v8a/memory 推送到有执行权限的设备目录
./memory PID
./memory PID 0x1234
```

示例只列出模块，可选读取指定地址的 int32，不自动查包名或请求 su。跨进程访问需要调用进程具有相应权限。传入当前有效 PID，进程重启后重建实例。

## 集成

包含 include/libfs.hpp，链接 lib/对应架构/libfs.a，通过 libfs::Memory(pid) 创建独立实例。refresh_modules() 更新快照，module() 按模块名、序号和区域查询，read()/write() 读写数据。写入前必须确认地址、类型和权限。

完整实例、回调、自定义驱动接入、短读与生命周期约定见 [libfs 使用说明](libfs-使用说明.md)。libfs 仅提供模块和读写，不解析 OUT/NORM 文件，也不包含扫描、网络和自动 Root 功能；动态链条需由调用方按结果偏移逐级解引用。

Cd 为可写数据段起点，Xa 为代码段起点，均不等同于整个 ELF 基址；模块区域必须与扫描结果一致。
