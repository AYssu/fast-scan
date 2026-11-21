#include "libfs.hpp"
#include "libfs_drivers.hpp"
#include <cstdio>
#include <cstring>

libfs::Memory init(const char *package);

int main() {
    try {
        // 包名只设置一次；子进程可填 com.example.app:worker。
        auto memory = init("com.example.app");

        // 模块名、序号、范围与扫描结果一致；可继续添加其他模块。
        auto xa = memory.get_module_base_str("libil2cpp.so", 1, "Xa");
        auto cd = memory.get_module_base_str("libil2cpp.so", 1, "Cd");
        auto cb = memory.get_module_base_str("libil2cpp.so", 1, "Cb");

        // lsp64：只解引用一次，适合自己逐级计算地址。
        auto pointer = memory.lsp64(cd + 0x100);
        auto value = memory.read_int(pointer + 0x20);

        // read_pointer：可写任意多组链条，最后一级只加偏移。
        auto address1 = memory.read_pointer(xa, {0x1234, 0x28, 0x18});
        auto address2 = memory.read_pointer(cd, {0x2340, 0x30, 0x20});
        auto address3 = memory.read_pointer(cb, {0x100, 0x18, 0x38});

        auto int_value = memory.read_int(address1);
        auto float_value = memory.read_float(address2);
        auto double_value = memory.read_double(address3);
        std::printf("单次指针读取：%d\n整数：%d\n浮点：%g\n双精度：%g\n",
                    value, int_value, float_value, double_value);

        // 按需取消注释；写入返回 bool，失败可能已经写入部分字节。
        // if (!memory.write_int(address1, 20)) return 1;
        // if (!memory.write_float(address2, 1.5f)) return 1;
        // if (!memory.write_double(address3, 2.5)) return 1;
        // 32 位目标使用 lsp32 / read_pointer32。
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "操作失败：%s\n", error.what());
        return 1;
    }
}

// 只在初始化中注入读写器；后面的用户代码不依赖具体驱动。
libfs::Memory init(const char *package) {
    const pid_t pid = libfs::find_pid(package);
    if (pid <= 0)
        throw std::system_error(-pid, std::generic_category(), "查找进程失败");

    auto driver = std::make_unique<libfs::SyscallDriver>();
    // 替换为 PreadDriver 或自己实现的驱动，后面的模块和链条代码不用改。
    if (!driver->set_pid(pid))
        throw std::runtime_error("初始化读写器失败");

    libfs::Memory memory(pid, std::move(driver));
    if (!memory)
        throw std::system_error(-memory.status(), std::generic_category(), "注入读写器失败");
    if (memory.refresh_modules() != 0)
        throw std::runtime_error("获取模块失败");
    return memory;
}
