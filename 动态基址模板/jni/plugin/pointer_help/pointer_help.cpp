/**
@author 阿夜
@content 简单的指针帮助类 满足大部分人的使用了
@date 2025/4/7
*/

#include "pointer_help.h"
// 使用内存读写库
#include <smemory.h>
#include <rv.h>

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

long pointer::get_pointer64(long start,std::vector<long> data) {
    if (data.empty())
        return {};
    for (auto i = 0; i < data.size()-1; ++i) {
        start = smemory::lsp<long>(start + data[i]);
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

uintptr_t pointer::lsp64(uintptr_t address) {
    // 可以自己去B4 什么什么的 自己看着来
    return smemory::lsp<uintptr_t>(address) & 0xFFFFFFFFFFFFFF;
}

int pointer::read_int(uintptr_t address) {
    return smemory::sread<int, uintptr_t>(address);
}

float pointer::read_float(uintptr_t address) {
    return smemory::sread<float, uintptr_t>(address);
}

double pointer::read_double(uintptr_t address) {
    return smemory::sread<double, uintptr_t>(address);
}

bool pointer::write_int(uintptr_t address, int value) {
    return smemory::swrite<int, uintptr_t>(address, value);
}


bool pointer::write_float(uintptr_t address, float value) {
    return smemory::swrite<float, uintptr_t>(address, value);
}

bool pointer::write_double(uintptr_t address, double value) {
    return smemory::swrite<double, uintptr_t>(address, value);
}

void pointer::get_buffer(char *buf, uintptr_t address)
{
    unsigned short buf16[16] = { 0 };
    smemory_readv::readv(address, buf16, 28);
    unsigned short *pTempUTF16 = buf16;
    char *pTempUTF8 = buf;
    char *pUTF8End = pTempUTF8 + 32;
    while (pTempUTF16 < pTempUTF16 + 28) {
        if (*pTempUTF16 <= 0x007F && pTempUTF8 + 1 < pUTF8End) {
            *pTempUTF8++ = (char) *pTempUTF16;
        } else if (*pTempUTF16 >= 0x0080 && *pTempUTF16 <= 0x07FF && pTempUTF8 + 2 < pUTF8End) {
            *pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        } else if (*pTempUTF16 >= 0x0800 && *pTempUTF16 <= 0xFFFF & pTempUTF8 + 3 < pUTF8End) {
            *pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
            *pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        } else {
            break;
        }
        pTempUTF16++;
    }
}