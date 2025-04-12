/**
@author 阿夜
@content 指针测试
@date 2025/4/7
*/

#include "pointer_vector.h"
// 使用内存读写库
#include <smemory.h>

uintptr_t pointer::get_pointer64(uintptr_t start,std::vector<uintptr_t> data) {
    if (data.empty())
        return {};
    for (auto i = 0; i < data.size()-1; ++i) {
        start = smemory::lsp<uintptr_t>(start + data[i]);
        if(start == 0)
            return {};
    }
    return start + data[data.size()-1];
}

uint32_t pointer::get_pointer32(uint32_t start,std::vector<uint32_t> data) {
    if (data.empty())
        return {};
    for (auto i = 0; i < data.size()-1; ++i) {
        start = smemory::lsp<uint32_t>(start + data[i]);
        if(start == 0)
            return {};
    }
    return start + data[data.size()-1];
}
