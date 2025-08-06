//
// Created by 阿夜 on 4/1/25.
//
#include "rv.h"

pid_t target_pid = -1;
inline thread_local iovec mem_local[1];
inline thread_local iovec mem_remote[1];

// 自定义读写内存 实现在对应的cpp 请按照规范 默认syscall
long smemory_readv::readv(uintptr_t addr, void *data, size_t size) {
    mem_local->iov_base = data;
    mem_local->iov_len = size;
    mem_remote->iov_base = reinterpret_cast<void *>(addr);
    mem_remote->iov_len = size;

    return syscall(SYS_process_vm_readv, target_pid, mem_local, 1, mem_remote, 1, 0);
}

long smemory_readv::writev(uintptr_t addr, void *data, size_t size) {
    mem_local->iov_base = data;
    mem_local->iov_len = size;
    mem_remote->iov_base = reinterpret_cast<void *>(addr);
    mem_remote->iov_len = size;
    return syscall(SYS_process_vm_writev, target_pid, mem_local, 1, mem_remote, 1, 0);
}

