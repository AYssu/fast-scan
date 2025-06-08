#include "feature.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <filesystem>
#include <smemory.h>

void feature::get_feature_code(uintptr_t address, int offset, const std::string &out)
{
    // 打印目标地址和偏移量
    std::cout << std::hex << "目标地址: " << address << std::dec << std::endl;
    std::cout << "偏移量: " << offset << std::endl;

    // 检查输出文件夹是否存在，如果不存在则创建
    std::filesystem::path out_path(out);
    if (!std::filesystem::exists(out_path))
    {
        std::filesystem::create_directories(out_path);
        std::cout << "创建输出文件夹: " << out << std::endl;
    }

    // 生成以当前时间戳和目标地址为名称的输出文件名
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
    std::string file_name = std::string(timestamp) + "_" + std::to_string(address) + ".txt";

    // 构建完整的输出文件路径
    std::string full_path = out + "/" + file_name;

    // 打开输出文件
    std::ofstream output_file(full_path);
    if (!output_file.is_open())
    {
        std::cerr << "无法打开输出文件: " << full_path << std::endl;
        return;
    }

    // 遍历从目标地址 - offset * 0x4 到目标地址 + offset * 0x4 的范围
    for (int i = -offset; i <= offset; ++i)
    {
        uintptr_t current_address = address + i * 0x4;

        auto read_int = smemory::sread<int>(current_address);
        auto read_float = smemory::sread<float>(current_address);
        auto read_double = smemory::sread<double>(current_address);

        // 如果 int 值为 0，则跳过该行
        if (read_int == 0)
        {
            continue;
        }

        // 格式化输出
        output_file << i  << " int: " << std::dec << read_int
                    << " float: " << read_float << read_float
                    << " double: " << read_double << read_double << std::endl;
    }

    // 关闭输出文件
    output_file.close();

    std::cout << "输出文件已保存到: " << full_path << std::endl;
}
