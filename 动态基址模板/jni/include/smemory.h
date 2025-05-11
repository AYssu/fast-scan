//
// Created by 阿夜 on 4/1/25.
//

#ifndef POINTER_SMEMORY_H
#define POINTER_SMEMORY_H

#include <iostream>
#include <functional>

namespace smemory {
    pid_t get_package_pid(const std::string &package);

    size_t get_module_base(const std::string &name, int index = 1, int ranger = 16);

    template<typename T, typename S>
    T sread(S addr);

    template<typename T, typename S>
    bool swrite(S addr, T value);

    template<class T>
    bool set_read(std::function<long(T address, void *buffer, size_t size)> func);

    template<class T>
    bool set_write(std::function<long(T address, void *buffer, size_t size)> func);

    template<typename T>
    T lsp(T addr);

} // namespace smemory

#endif //POINTER_SMEMORY_H
