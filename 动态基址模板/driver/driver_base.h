#ifndef DRIVER_BASE_H
#define DRIVER_BASE_H

#include <cstdint>

/**
 * 驱动基类接口
 * 所有驱动类都应该继承这个基类并实现这些虚函数
 */
class driver_base {
public:
    virtual ~driver_base() = default;
    
    // 设置目标进程PID
    virtual bool set_pid(pid_t pid) = 0;
    
    // 读取内存
    virtual bool read(uintptr_t address, void* buffer, size_t size) = 0;
    
    // 写入内存
    virtual bool write(uintptr_t address, void* buffer, size_t size) = 0;
    
    // 模板方法：读取指定类型数据
    template <typename T>
    T read(uintptr_t address) {
        T res{};
        if (this->read(address, &res, sizeof(T)))
            return res;
        return {};
    }
    
    // 模板方法：写入指定类型数据
    template <typename T>
    bool write(uintptr_t address, T value) {
        return this->write(address, &value, sizeof(T));
    }
    
    // 获取模块基址（可选实现）
    virtual uintptr_t get_module_base(const char* name) {
        return 0;
    }
    
protected:
    pid_t target_pid = 0;
};

#endif // DRIVER_BASE_H

