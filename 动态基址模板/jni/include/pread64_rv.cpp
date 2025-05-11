//
// Created by 阿夜 on 4/1/25.
//
#include "rv.h"

pid_t target_pid = -1;

class mem_pread {
private:
    int mem_handle = -1;
public:

    bool get_handle(pid_t pid) {
        if (mem_handle > 0)
            return true;
        std::string path = "/proc/" + std::to_string(pid) + "/mem";
        mem_handle = open(path.c_str(), O_RDWR);
        if (-1 == mem_handle) {
            std::cout << "打开进程mem失败" << std::endl;
            return false;
        }
        return mem_handle > 0;
    }
    auto get_handle() {
        return mem_handle;
    }
    ~mem_pread() {
        if (mem_handle > 0)
            close(mem_handle);
    }
};

auto tool = new mem_pread();

// 最好定义一个方法专门去处理 handle 和close 的逻辑 这边写得比较草率
long smemory_readv::readv(uintptr_t addr, void *data, size_t size) {
    if (!tool->get_handle(target_pid))
        return -1;
    return pread64(tool->get_handle(), data, size, static_cast<off64_t>(addr));
}

long smemory_readv::writev(uintptr_t addr, void *data, size_t size) {
    if (!tool->get_handle(target_pid))
        return -1;
    return pwrite64(tool->get_handle(), data, size, static_cast<off64_t>(addr));
}