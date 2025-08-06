//
// Created by 阿夜 on 4/1/25.
//

#ifndef POINTER_SMEMORY_H
#define POINTER_SMEMORY_H

#include <cstdint>
#include <vector>
#include <iostream>
#include <functional>

namespace smemory {
    /**
     * 获取进程pid
     * @param package 安装包名
     * @return pid
     */
    pid_t get_package_pid(const std::string &package);

    /**
     * 获取模块基址
     * @param name 模块名称
     * @param index 模块下标
     * @param ranger 模块范围(Xa=16384,Cd=16,Ch=5)
     * @return
     */
    size_t get_module_base(const std::string &name, int index = 1, int ranger = 16);

    /**
        * 获取模块偏移
        * @param name 模块名称
        * @param index 下标
        * @param ranger 范围 (Cd=16, Cb=8, Xa=16384)
        * @return 模块起始地址
        */
    size_t get_module_base_str(const std::string &name, int index = 1, const std::string &ranger = "Cd");
    /**
     * 读取内存
     * @tparam T 返回类型
     * @tparam S 指针类型
     * @param addr 内存地址
     * @return 读取到的数据
     */
    template<typename T, typename S>
    T sread(S addr);

    /**
     * 写入内存
     * @tparam T 写入类型
     * @tparam S 指针类型
     * @param addr 内存地址
     * @param value 写入数据
     * @return 是否写入成功
     */
    template<typename T, typename S>
    bool swrite(S addr, T value);

    /**
     * 设置读取内存方法
     * @tparam T 内存函数类型
     * @param func 读写函数
     * @return 是否设置成功
     */
    template<class T>
    bool set_read(std::function<long(T address, void *buffer, size_t size)> func);

    /**
     * 设置写入内存方法
     * @tparam T 内存函数类型
     * @param func 写入函数
     * @return 是否设置成功
     */
    template<class T>
    bool set_write(std::function<long(T address, void *buffer, size_t size)> func);

    /**
     * 获取指针
     * @tparam T 指针类型
     * @param addr 内存地址
     * @return 指针
     */
    template<typename T>
    T lsp(T addr);

} // namespace smemory


namespace pointer {

    uintptr_t get_pointer64(uintptr_t start, std::vector<uintptr_t> data);// 获取指针数据

    long get_pointer64(long start, std::vector<long> data);//  跳转指针

    uint32_t get_pointer32(uint32_t start, std::vector<uint32_t> data);// 或者32位指针数据

    uintptr_t lsp64(uintptr_t address);// 跳转指针

    int read_int(uintptr_t address);// 获取int类型

    float read_float(uintptr_t address);// 获取float类型

    double read_double(uintptr_t address); // 获取double类型

    uint16_t read_word(uintptr_t address); // 获取word类型

    uint64_t read_qword(uintptr_t address); // 获取qword类型



    uint8_t  read_char(uintptr_t address); // 获取char类型

    bool write_int(uintptr_t address, int value);// 写入int类型

    bool write_float(uintptr_t address, float value);// 写入float类型

    bool write_double(uintptr_t address, double value);// 写入double类型

    bool write_char(uintptr_t address, uint8_t  value); // 写入char类型

    bool write_word(uintptr_t address, uint16_t value); // 写入word类型

    bool write_qword(uintptr_t address, uint64_t value); //

    void get_buffer(char *buf, uintptr_t address); // 获取文本数据


}// namespace pointer

#endif //POINTER_SMEMORY_H
