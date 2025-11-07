#ifndef DRIVER_SYSCALL_H
#define DRIVER_SYSCALL_H

#include "driver_base.h"
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

/**
 * Syscall 驱动
 * 使用 process_vm_readv/process_vm_writev 系统调用读写进程内存
 * 这是 Linux 提供的跨进程内存访问接口，效率较高
 */
class syscall_driver : public driver_base {
public:
    syscall_driver() {
        std::cout << "[+] Syscall 初始化成功" << std::endl;
    }
    
    ~syscall_driver() override {
        std::cout << "[-] Syscall 关闭成功" << std::endl;
    }
    
    bool set_pid(pid_t pid) override {
        target_pid = pid;
        return target_pid > 0;
    }
    
    bool read(uintptr_t address, void* buffer, size_t size) override {
        if (target_pid <= 0) {
            std::cout << "[-] Syscall : 目标PID无效" << std::endl;
            return false;
        }
        
        // 设置本地和远程 iovec 结构
        struct iovec local[1];
        struct iovec remote[1];
        
        local[0].iov_base = buffer;
        local[0].iov_len = size;
        
        remote[0].iov_base = reinterpret_cast<void*>(address);
        remote[0].iov_len = size;
        
        // 调用 process_vm_readv 系统调用
        ssize_t result = syscall(SYS_process_vm_readv, target_pid, local, 1, remote, 1, 0);
        
        return result == static_cast<ssize_t>(size);
    }
    
    bool write(uintptr_t address, void* buffer, size_t size) override {
        if (target_pid <= 0) {
            std::cout << "[-] Syscall : 目标PID无效" << std::endl;
            return false;
        }
        
        // 设置本地和远程 iovec 结构
        struct iovec local[1];
        struct iovec remote[1];
        
        local[0].iov_base = buffer;
        local[0].iov_len = size;
        
        remote[0].iov_base = reinterpret_cast<void*>(address);
        remote[0].iov_len = size;
        
        // 调用 process_vm_writev 系统调用
        ssize_t result = syscall(SYS_process_vm_writev, target_pid, local, 1, remote, 1, 0);
        
        return result == static_cast<ssize_t>(size);
    }
};

#endif // DRIVER_SYSCALL_H

