/**
@author 阿夜
@content 特征码获取
@date 2025/5/14
*/

#ifndef FEATURE_CODE_H
#define FEATURE_CODE_H

#include <iostream>

namespace feature {
    // 获取特征码 
    void get_feature_code(uintptr_t address = 0x0,int offset = 500000,const std::string &out = "");
}// namespace test

#endif//FEATURE_CODE_H
