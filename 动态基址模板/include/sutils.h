//
// Created by AYssu on 2025/05/07
//
#ifndef Sea_3_H
#define Sea_3_H

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include "readline/history.h"
#include "readline/readline.h"
#include <regex>
#include "sutils.h"
#include <sstream>
#include <string>
#include <unordered_map>

#include <sstream>

#include <cstdio>
#include <cstring>
#include <set>
#include <sys/wait.h>
#include <unordered_map>
#include <vector>

namespace sutils {
    /**
     * 读取文件
     * @param path 文件路径
     * @return 文件内容
     */
    std::string read_file(const std::string &path);

    /**
     * 写入文件
     * @param path 文件路径
     * @param str 文件内容
     * @return 写入的字节数
     */
    size_t write_file(const std::string &path, const std::string &str);

    /**
     * 将字符串转换为整数向量
     * @param input 字符串
     * @return 整数向量
     */
    std::vector<int> convert_to_int_vector(const std::string &input);
}

namespace custom_sutils {
    /**
     * 获取用户输入的整数
     * @param msg 提示信息
     * @return 用户输入的整数
     */
    int get_input_int(const std::string &msg);

    /**
     * 获取用户输入的字符串
     * @param msg
     * @return 用户输入的字符串
     */
    std::string get_input_string(const std::string &msg);

    /**
     * 获取用户输入的无符号整数
     * @param msg 提示信息
     * @return 用户输入的无符号整数
     */
    uintptr_t get_input_uintptr_t(const std::string &msg);// 获取用户输入的无符号整数
}
#endif//Sea_3_H
