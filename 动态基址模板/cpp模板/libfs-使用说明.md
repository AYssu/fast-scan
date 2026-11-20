# libfs

面向 Android/Linux 的轻量进程内存与模块 SDK，C++17 实现，提供 C ABI 和 C++ 独立实例封装。当前版本 0.1.0。

SeaAlpha 的 `driver_base` 对象和读写回调适合作为接入方式；但它的 PID、回调和模块列表是全局静态状态，完整静态库还会合并网络、SSL、readline、ncurses 等组件。libfs 单独保留读写和模块能力，每个实例各自持有 PID、读写器、资源及模块快照。

## 能力

- 内置 `process_vm_readv/writev` 和 `/proc/PID/mem` 的 `pread/pwrite` 两种读写方式，创建时明确选择。
- 接入自己初始化好的读写器实例，或通过 C 回调接入其他语言、内核驱动。库不内置特定驱动协议。
- 查询模块起止地址、文件偏移、权限、名称、范围与序号，支持 `Cd`、`Cb`、`Xa`、`Cdro`、`Cdx`。
- 复用 core-fs 的 ELF、APK 内直接加载 SO 与 BSS 识别逻辑；模块序号按名称与范围分别从 1 开始，地址按 maps 顺序排列。
- 完成合法短读/短写；失败时报告实际完成前缀，不把没有传满的数据算作成功。C++ 类型读取失败时保留调用者原值。

不依赖网络、授权服务、JSON、扫描引擎或终端界面。不会执行 `su`，不会自动切换读写器，不向终端打印。实例的读写权限来自调用进程的现有权限。

## 分发内容与链接

当前模板 include/ 提供 libfs.h、libfs.hpp；lib/arm64-v8a/libfs.a 与 lib/x86_64/libfs.a 分别对应两种 Android 架构，最低 API 24。仅提供 64 位宿主库；可读取目标的 32 位普通数据及 uint32_t 指针。

使用 Android NDK、C++17，按模板 README 执行一键构建脚本。jni/Android.mk 已声明预编译静态库，jni/Application.mk 使用 c++_static，无需额外携带 libfs.so。也可在自己的 CMake 工程创建 IMPORTED STATIC 目标，指定对应架构库与 include/，链接 C++ 运行库及线程依赖。

此分发只包含 SDK 头文件、静态库和调用示例，不包含 libfs 源码构建工程。示例 memory PID 列出模块，memory PID 0x地址 读取一个 int32。

## C++ 实例

```cpp
#include <libfs.hpp>
#include <cstdio>

int inspect(pid_t pid, uint64_t address) {
    libfs::Memory memory(pid); // 默认 process_vm；可传 FS_PROC_MEM
    if (!memory) return memory.status();

    int error = memory.refresh_modules();
    if (error) return error;

    fs_module module{};
    error = memory.module("libgame.so", module, 1, FS_CD);
    if (error) return error;
    std::printf("%s[%u][%s] 0x%llx\n", module.name, module.index,
                fs_range_label(module.range), (unsigned long long)module.start);

    int32_t value = 0;
    if (!memory.read(address, value)) return -1;
    // address 必须是你已确认可写的目标数据地址；写失败可能已经写入部分字节。
    if (!memory.write(address, int32_t{456})) return -1;
    return 0;
}
```

`Memory` 不可复制，可以移动，销毁时关闭自己的文件描述符或读写器。`status()` 表示实例创建状态；需要具体读写错误时使用接收 buffer/size 的重载，其返回值为字节数或负 errno。多个实例可访问不同 PID，互不覆盖配置。

模块地址是对应内存段的起点：`[Cd]` 是可写段起点，`[Xa]` 是代码段起点，均不应当作整个 ELF 的统一基址。`file_offset` 是该段在映射文件中的偏移，APK 映射时包含 ZIP 内的偏移。Cb 返回 `libgame.so:bss`；查询时也接受 `"libgame.so"` 加 `FS_CB`。

## 接入现有读写器对象

现有 SeaAlpha 驱动可用很薄的一层适配，无需继承库内部类，也不需要搬入原来的网络/UI 代码。下面的 `ExistingDriver` 代表调用方现有的、已经初始化到目标 PID 的驱动类型：

```cpp
class Adapter final : public libfs::Reader {
    std::unique_ptr<ExistingDriver> driver_;
public:
    explicit Adapter(std::unique_ptr<ExistingDriver> driver)
        : driver_(std::move(driver)) {}
    bool read(uint64_t address, void* buffer, size_t size) override {
        return driver_->read(address, buffer, size);
    }
    bool write(uint64_t address, const void* buffer, size_t size) override {
        return driver_->write(address, const_cast<void*>(buffer), size);
    }
};

auto driver = std::make_unique<ExistingDriver>();
if (!driver->set_pid(pid)) { /* 处理初始化失败 */ }
libfs::Memory memory(pid, std::make_unique<Adapter>(std::move(driver)));
```

适配器负责驱动对象的初始化和释放。布尔接口的 `true` 必须表示**整个请求**已完成；不能把实际返回字节数直接转成 bool。需要处理短读的驱动优先使用下面的 C 回调，明确返回字节数。库不会猜测驱动是 bool、字节数还是零成功约定。

## C ABI 与字节数回调

完整声明在 `include/libfs.h`。`fs_open` 创建内置读写器；`fs_open_callbacks` 接收 `user/read/write/destroy`，支持一个实例一个对象。

- 回调返回传输字节数，失败返回 `-EFAULT`、`-EACCES` 等**负 errno**；不是 `false/true`，也不是 POSIX 的 `-1` 加 errno 约定。
- 短读/短写会按已完成长度继续，`-EINTR` 重试，零字节表示本次未取得进展，返回错误或已完成前缀。只读实例可将 write 设为 NULL。
- `fs_open_callbacks` 成功后才转移 user 的释放责任；失败时仍由调用者释放。destroy 可为空，由外部管理对象，但必须活到实例销毁后。
- 状态函数返回 0 或负 errno；`fs_read/fs_write` 返回完成字节数或负 errno。写入失败不保证回滚。
- 一个实例的操作串行化；不同实例可并行。回调不要重入同一个实例，关闭实例前应结束所有使用它的任务，回调不得抛异常。
- `fs_pid`、`fs_version`、`fs_range_label` 是只读查询。`fs_close(NULL)` 安全。

`fs_refresh_modules` 显式建立/更新快照，查询不会偷偷读取 maps；刷新失败保留上一次成功快照。未刷新时查询返回 `-EAGAIN`；找不到模块返回 `-ENOENT`。调用者应在目标 dlopen/dlclose 后重新刷新。目标退出/重启后应关闭旧实例，用新 PID 创建实例；库不跟踪进程重启或自动重新绑定。

## 模块识别边界

模块信息来自 `/proc/PID/maps`，需要调用者有权限读取。自定义内核读写不会自动提供模块列表或绕过 maps 权限。

APK 内 SO 识别要求可访问 APK、未压缩且对齐的原生库条目；ZIP64/多磁盘 ZIP 暂不支持。ELF 身份使用 maps 中的 inode/设备号校验，支持 `/proc/PID/root` 与 `map_files`，不以同名文件代替真实映射。普通 SO 文件无法访问时仍可按 maps 路径查询其文件段；APK 成员名及匿名 BSS 则必须有可验证的 ELF 依据，不盲猜相邻匿名段归属。overlayfs 的设备号不一致且 `map_files` 无权限时，匿名 BSS/APK 成员可能无法识别。

相比直接复制整个 maps 段，本库把合并匿名 VMA 中的 BSS 截取到 ELF 验证的边界。无法唯一确认归属时省略该 BSS，不把无关匿名内存算进模块。

当前模块名称和路径字段上限 255 字节，超长 maps 路径会使刷新返回 `-ENAMETOOLONG`，保留旧快照；不会静默截断后返回错误模块。地址保留原值，库不自动去除架构标签位或猜测指针宽度。

## 配套文件验证

本分发已使用配套两架构静态库与头文件构建 Android 示例。构建通过不等于所有设备具备跨进程权限；实际访问取决于调用进程。
