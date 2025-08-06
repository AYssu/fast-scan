//
// Created by AYssu on 2025/01/04.
//

#ifndef POINTER_RV_H
#define POINTER_RV_H
#include <cstdint>       // 用于定义固定宽度的整数类型
#include <cstddef>       // 用于定义标准类型，如 size_t
#include <cstring>       // 用于操作 C 风格字符串
#include <iostream>      // 用于输入输出操作
#include <unordered_map> // 用于哈希表
#include <vector>        // 用于动态数组
#include <functional>    // 用于函数对象和绑定器

#include <dirent.h>      // 用于目录操作
#include <fcntl.h>       // 用于文件控制操作
#include <sys/mman.h>    // 用于内存映射
#include <sys/stat.h>    // 用于文件状态操作
#include <sys/syscall.h> // 用于系统调用
#include <sys/uio.h>     // 用于 I/O 操作
#include <sys/user.h>    // 用于用户空间操作，例如 PAGE_SIZE
#include <unistd.h>      // 用于标准符号常量和类型
#include <cstdio>        // 用于 C 风格输入输出操作
#include <cstdlib>       // 用于标准库函数


// 全局的 进程PID
extern pid_t target_pid;
// uintptr_t 64位 改成 uint32_t 32位 对应的读写cpp也需要修改
namespace smemory_readv {

    // 自定义读写内存 实现在对应的cpp 请按照规范 默认syscall
    long readv(uintptr_t addr, void *data, size_t size);
    long writev(uintptr_t addr, void *data, size_t size);

}; // namespace smemory_readv
#endif //POINTER_RV_H
