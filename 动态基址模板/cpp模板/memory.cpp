#include "smemory.h"
#include "driver/driver_syscall.h"

using namespace pointer;
bool init(const std::string &package);

int main(int argc, char *argv[]) {

    init("com.example.app");

    auto module_base = smemory::get_module_base_str("libil2cpp.so", 1, "Xa" );

    std::cout << "模块基址: " << std::hex << module_base << std::dec << std::endl;
    auto read_i = read_int(module_base + 0x8);
    auto read_f = read_float(module_base + 0x8);
    auto read_d = read_double(module_base + 0x8);

    std::cout << "读取64 F: " << read_f << std::endl;

    std::cout << "读取64 D: " << read_i << std::endl;

    std::cout << "读取64 E: " << read_d << std::endl;

    /**
        write_int(module_base, 20);// 64写入整数
        write_float(module_base , 20); // 写入浮点
        write_double(module_base , 20); // 写入E
    **/


    auto read_pointer = lsp64(module_base + 0x10);
    std::cout << "读取指针: 0x" << std::hex << read_pointer << std::dec << std::endl;

    std::vector<uintptr_t> pointers = { 0x1234, 0x245, 0x123 };
    auto read_pointer64 = pointer::get_pointer64(module_base, pointers);
    std::cout << "使用插件读取指针: 0x" << std::hex << read_pointer64 << std::dec << std::endl;

    return 0;
}


bool init(const std::string &package) {// 获取进程PID和模块基址 by 阿夜
    auto pid = smemory::get_package_pid(package);
    std::cout << "进程PID: " << pid << std::endl;
    // uintptr_t=64 uint32_t=32
    static auto driver_mgr = new syscall_driver();  
    auto read_flag = smemory::set_read<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
        return driver_mgr->read(address, data, size);
    });// 调用自定义读写 实现低耦合 理论支持全读写 即使未来出了什么无敌读写也能快速适配

    auto write_flag = smemory::set_write<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
        return driver_mgr->write(address, data, size);
    });

    // 设置完成 下次静态库调用的为编译的读写 小白啥也不用管 默认即可
    if (!read_flag || !write_flag) {
        std::cout << "设置自定义读写失败" << std::endl;
        return false;
    }

    // uintptr_t=64 uint32_t=32 注意 只适配64可读写32
    // 目前支持自定义读写 syscall pread64 kernel 均自己实现
    if (pid <= 0)
        return false;
    driver_mgr->set_pid(pid);
    return true;
}
