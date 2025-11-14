#ifndef DRIVER_PREAD_H
#define DRIVER_PREAD_H

#include "driver_base.h"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <iostream>

/**
 * Pread64 驱动
 * 使用 pread64/pwrite64 系统调用读写进程内存
 * 通过 /proc/[pid]/mem 文件进行操作
 */
class pread_driver : public driver_base {
private:
    int mem_handle = -1;
    
    // 打开目标进程的 mem 文件
    bool open_mem_handle() {
        if (mem_handle > 0) {
            return true;
        }
        
        if (target_pid <= 0) {
            std::cout << "[-] Pread: 目标PID无效" << std::endl;
            return false;
        }
        
        std::string path = "/proc/" + std::to_string(target_pid) + "/mem";
        mem_handle = open(path.c_str(), O_RDWR);
        
        if (mem_handle == -1) {
            std::cout << "[-] Pread: 无法打开 " << path << std::endl;
            return false;
        }
        
        return true;
    }
    
    // 关闭 mem 文件句柄
    void close_mem_handle() {
        if (mem_handle > 0) {
            close(mem_handle);
            mem_handle = -1;
        }
    }

public:
    pread_driver() : mem_handle(-1) {
        std::cout << "[+] Pread驱动初始化成功" << std::endl;
    }
    
    ~pread_driver() override {
        close_mem_handle();
        std::cout << "[-] Pread驱动关闭成功" << std::endl;
    }
    
    bool set_pid(pid_t pid) override {
        // 如果切换了PID，需要关闭旧的句柄
        if (target_pid != pid && mem_handle > 0) {
            close_mem_handle();
        }
        
        target_pid = pid;
        return target_pid > 0;
    }
    
    bool read(uintptr_t address, void* buffer, size_t size) override {
        if (!open_mem_handle()) {
            return false;
        }
        
        ssize_t result = pread64(mem_handle, buffer, size, static_cast<off64_t>(address));
        return result == static_cast<ssize_t>(size);
    }
    
    bool write(uintptr_t address, void* buffer, size_t size) override {
        if (!open_mem_handle()) {
            return false;
        }
        
        ssize_t result = pwrite64(mem_handle, buffer, size, static_cast<off64_t>(address));
        return result == static_cast<ssize_t>(size);
    }
};

#endif // DRIVER_PREAD_H

