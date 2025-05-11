/**
@author 阿夜
@content 指针测试
@date 2025/4/7
*/

#ifndef POINTER_POINTER_VECTOR_H
#define POINTER_POINTER_VECTOR_H

#include <cstdint>
#include <vector>
namespace pointer {
    // 或者64位指针数据
    uintptr_t get_pointer64(uintptr_t start, std::vector<uintptr_t> data);

    long get_pointer64(long start, std::vector<long> data);

    // 或者32位指针数据
    uint32_t get_pointer32(uint32_t start, std::vector<uint32_t> data);

}// namespace pointer

#endif//POINTER_POINTER_VECTOR_H
