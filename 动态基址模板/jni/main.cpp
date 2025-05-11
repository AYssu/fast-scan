#include "include/smemory.h"
#include "include/rv.h"
#include "plugin/pointer_vector/pointer_vector.h"
// 可以定义一些自己看得懂的函数 如果决定模板函数看不懂的化
uintptr_t lsp64(uintptr_t address);

uint32_t lsp32(uint32_t address);

bool init(const std::string &package);


int main(int argc, char *argv[]) {

    init("gg.pointers");

    // 模块名 index range(16=cd 8=cb 16384=xa)
    auto module_base = smemory::get_module_base("libgame.so", 2, 16);
    std::cout << "模块基址: " << std::hex << module_base << std::dec << std::endl;

    auto read_int = smemory::sread<int, uintptr_t>(module_base+ 0x8);
    std::cout << "读取64整数: " << read_int << std::endl;
    auto read_float = smemory::sread<float, uintptr_t>(module_base + 0x8);
    std::cout << "读取64浮点: " << read_float << std::endl;

    smemory::swrite<int, uintptr_t>(module_base , 20); // 64写入整数
//    smemory::swrite<float, uintptr_t>(module_base , 20); // 写入浮点

    auto read_pointer = lsp64(module_base + 0x10);
    std::cout << "读取指针: 0x" << std::hex << read_pointer  << std::dec << std::endl;

    std::vector<uintptr_t > pointers = {0x8,0x140,0x80,0x9F8,0xBA0,0x790,0xEA8,0xC8,0x1A0,0x1E8,0x37C};
    auto read_pointer64 = pointer::get_pointer64(module_base,pointers);
    std::cout << "使用插件读取指针: 0x" << std::hex << read_pointer64  << std::dec << std::endl;


    return 0;
}



bool init(const std::string &package)
{  // 获取进程PID和模块基址 by 阿夜
    auto pid = smemory::get_package_pid(package);
    target_pid = pid; // 注意target_pid 为读写内PID 一定要写不然自定义读写没进程ID会失效
    // uintptr_t=64 uint32_t=32
    auto read_flag = smemory::set_read<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
        return smemory_readv::readv(address, data, size);
    }); // 调用自定义读写 实现低耦合 理论支持全读写 即使未来出了什么无敌读写也能快速适配

    auto write_flag = smemory::set_write<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
        return smemory_readv::writev(address, data, size);
    });

    // 设置完成 下次静态库调用的为编译的读写 小白啥也不用管 默认即可
    if (!read_flag || !write_flag) {
        std::cout << "设置自定义读写失败" << std::endl;
        return false;
    }

    // uintptr_t=64 uint32_t=32 注意 只适配64可读写32
    // 目前支持自定义读写 syscall pread64 kernel 均自己实现
    if (pid<=0)
        return false;
    std::cout << "进程PID: " << pid << std::endl;
    return true;
}

uintptr_t lsp64(uintptr_t address)
{
    // 可以自己去B4 什么什么的 自己看着来
    return smemory::lsp<uintptr_t>(address);
}

uint32_t lsp32(uint32_t address)
{
    return smemory::lsp<uint32_t>(address);
}