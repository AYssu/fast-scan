//
// Created by 阿夜 on 4/1/25.
//

#ifndef POINTER_SMEMORY_H
#define POINTER_SMEMORY_H

#include <iostream>
#include <functional>

namespace smemory {
    /**
     * 获取包pid
     * @param package 安装包名称
     * @return 进程PID
     */
    pid_t get_package_pid(const std::string &package);

    /**
     * 获取模块偏移
     * @param name 模块名称
     * @param index 下标
     * @param ranger 范围 (Cd=16, Cb=8, Xa=16384)
     * @return 模块起始地址
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
     * @tparam T 指针类型
     * @tparam S 获取数据类型
     * @param addr 地址
     * @return 获取的数据
     */
    template<typename T, typename S>
    T sread(S addr);

    /**
     * 写入内存
     * @tparam T 指针类型
     * @tparam S 数据类型
     * @param addr 地址
     * @param value 写入的数据
     * @return
     */
    template<typename T, typename S>
    bool swrite(S addr, T value);

    /**
     * 设置读取内存函数
     * @tparam T 指针类型
     * @param func 读取内存函数
     * @return 是否设置成功
     */
    template<class T>
    bool set_read(std::function<long(T address, void *buffer, size_t size)> func);

    /**
     * 设置写入内存函数
     * @tparam T 指针类型
     * @param func 写入内存函数
     * @return 是否设置成功
     */
    template<class T>
    bool set_write(std::function<long(T address, void *buffer, size_t size)> func);

    /**
     * 获取指针
     * @tparam T 指针类型
     * @param addr 地址
     * @return 指针地址
     */
    template<typename T>
    T lsp(T addr);

} // namespace smemory

#endif //POINTER_SMEMORY_H
