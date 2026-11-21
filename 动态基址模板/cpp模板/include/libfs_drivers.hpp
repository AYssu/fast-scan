#pragma once
#include "libfs.hpp"

namespace libfs {

// Extension point: own resources in the driver; true means the FULL transfer succeeded.
class Driver : public Reader {
public:
    virtual bool set_pid(pid_t pid) = 0;
};

// Adapters reuse the SDK's tested exact-transfer implementations and descriptor lifecycle.
class NativeDriver : public Driver {
    fs_backend backend_;
    std::unique_ptr<Memory> target_;
protected:
    explicit NativeDriver(fs_backend backend) : backend_(backend) {}
public:
    bool set_pid(pid_t pid) override {
        auto next = std::make_unique<Memory>(pid, backend_);
        if (!*next) {
            target_.reset();
            return false;
        }
        target_ = std::move(next);
        return true;
    }
    bool read(uint64_t address, void *buffer, size_t size) override {
        return size <= static_cast<size_t>(std::numeric_limits<ssize_t>::max()) &&
            target_ && target_->read(address, buffer, size) == static_cast<ssize_t>(size);
    }
    bool write(uint64_t address, const void *buffer, size_t size) override {
        return size <= static_cast<size_t>(std::numeric_limits<ssize_t>::max()) &&
            target_ && target_->write(address, buffer, size) == static_cast<ssize_t>(size);
    }
};

class SyscallDriver final : public NativeDriver {
public:
    SyscallDriver() : NativeDriver(FS_PROCESS_VM) {}
};

class PreadDriver final : public NativeDriver {
public:
    PreadDriver() : NativeDriver(FS_PROC_MEM) {}
};

} // namespace libfs
