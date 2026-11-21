# libfs C++ 指针链模板

libfs 0.1.1 静态库，支持 Android arm64-v8a、x86_64，最低 API 24。

## 修改 main.cpp 中的用户代码

包名只在 `init("com.example.app")` 填写一次。初始化时注入读写器实例，创建 Memory 后，可以自由添加多组模块、指针链条和数值读写，不局限于一组配置。

```cpp
auto xa = memory.get_module_base_str("libil2cpp.so", 1, "Xa");
auto cd = memory.get_module_base_str("libil2cpp.so", 1, "Cd");
auto cb = memory.get_module_base_str("libil2cpp.so", 1, "Cb");
auto single = memory.lsp64(cd + 0x100);
auto address1 = memory.read_pointer(xa, {0x1234, 0x28, 0x18});
auto address2 = memory.read_pointer(cb, {0x100, 0x18, 0x38});
auto value1 = memory.read_int(address1);
auto value2 = memory.read_float(address2);
```

模块范围直接填写 `"Xa"`、`"Cd"`、`"Cb"`，也支持 `"Cdro"`、`"Cdx"`。模块名、序号、范围与扫描结果一致，可以按需查询其他 SO。

`lsp64` 只读取一次 64 位指针；`read_pointer` 根据偏移链计算最终地址，最后一级不解引用。32 位目标使用 `lsp32` / `read_pointer32`。

读写可用 `read_int/read_float/read_double` 与 `write_int/write_float/write_double`。读取和模块查询失败会抛异常，由 main 的 catch 显示错误，不用数值 0 假装成功；写入返回 bool。需要无异常接口时，原来的 `read/write/pointer64/module` 仍保留。

## 构建与运行

```sh
export ANDROID_NDK=/你的/android-ndk
bash 一键构建脚本.sh
# 将 libs/对应架构/memory 推送到有执行权限的设备目录
./memory
```

跨进程访问需要相应权限，模板不会自动申请 Root。模板在 `init` 中创建 `SyscallDriver`，先调用 `set_pid`，再通过 `Memory(pid, std::move(driver))` 注入。要用 pread64，只替换为 `PreadDriver`；后面的用户代码保持不变。自定义驱动继承 `libfs::Driver`，实现 `set_pid/read/write` 即可接入，`read/write` 返回 true 必须表示整个请求完成。实例销毁时自动释放驱动及其资源，失败的重新绑定不会继续读写旧 PID。模块枚举仍需要读取目标 maps 的权限。

存在多个同名进程时查 PID 返回 `-EEXIST`，请明确改用目标 PID 创建实例，避免误选分身。目标重启后重新运行程序。

## 链条规则与旧模板一致

`{0x18, 0x2c, 0x38}` 表示 `[[模块起点 + 0x18] + 0x2c] + 0x38`。最后一级只加偏移，不解引用，随后 `read` 获取该地址的数值。空链、空指针、读取失败或地址溢出返回失败，不把失败当作数值 0。

模块起点是所选区域起点：Cd 为可写数据段、Xa 为代码段，不等同于整个 ELF 基址。范围必须与扫描结果保持一致。

完整实例、自定义驱动接入、短读及生命周期约定见 [libfs 使用说明](libfs-使用说明.md)。原来的 PID/模块枚举诊断工具仍在 SDK 的 `fs_inspect`，不再作为用户指针模板。
