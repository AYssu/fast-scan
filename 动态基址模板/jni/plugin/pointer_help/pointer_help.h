/**
@author 阿夜
@content 简单的 指针操作类
@date 2025/4/7
*/

#ifndef POINTER_POINTER_HELP_H
#define POINTER_POINTER_HELP_H

#include <cstdint>
#include <vector>
namespace pointer {

    uintptr_t get_pointer64(uintptr_t start, std::vector<uintptr_t> data);// 获取指针数据

    long get_pointer64(long start, std::vector<long> data);//  跳转指针

    uint32_t get_pointer32(uint32_t start, std::vector<uint32_t> data);// 或者32位指针数据

    uintptr_t lsp64(uintptr_t address);// 跳转指针

    int read_int(uintptr_t address);// 获取D类型

    float read_float(uintptr_t address);// 获取F类型

    double read_double(uintptr_t address);

    bool write_int(uintptr_t address, int value);// 写入D类型

    bool write_float(uintptr_t address, float value);// 写入F类型

    bool write_double(uintptr_t address, double value);// 写入D类型

    void get_buffer(char *buf, uintptr_t address); // 获取文本数据

}// namespace pointer

#endif//POINTER_POINTER_HELP_H
