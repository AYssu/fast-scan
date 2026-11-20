#pragma once
#include "libfs.h"
#include <cerrno>
#include <memory>
#include <type_traits>

namespace libfs {

// Compatible in shape with SeaAlpha's driver_base: the driver instance owns
// initialization/resources and must report whether the FULL request succeeded.
class Reader {
public:
    virtual ~Reader() = default;
    virtual bool read(uint64_t address, void *buffer, size_t size) = 0;
    virtual bool write(uint64_t, const void *, size_t) { return false; }
};

class Memory {
    struct Deleter { void operator()(fs_context *p) const { fs_close(p); } };
    std::unique_ptr<fs_context, Deleter> context_;
    int status_ = -EINVAL;
public:
    explicit Memory(pid_t pid, fs_backend backend = FS_PROCESS_VM) {
        fs_context *raw = nullptr;
        status_ = fs_open(pid, backend, &raw);
        context_.reset(raw);
    }
    Memory(pid_t pid, std::unique_ptr<Reader> reader) {
        fs_callbacks callbacks{};
        callbacks.user = reader.get();
        callbacks.read = [](void *p, pid_t, uint64_t a, void *b, size_t n) -> ssize_t {
            return static_cast<Reader *>(p)->read(a, b, n) ? static_cast<ssize_t>(n) : -EIO;
        };
        callbacks.write = [](void *p, pid_t, uint64_t a, const void *b, size_t n) -> ssize_t {
            return static_cast<Reader *>(p)->write(a, b, n) ? static_cast<ssize_t>(n) : -EIO;
        };
        callbacks.destroy = [](void *p) { delete static_cast<Reader *>(p); };
        if (!reader) return;
        fs_context *raw = nullptr;
        status_ = fs_open_callbacks(pid, &callbacks, &raw);
        if (status_ == 0) reader.release();
        context_.reset(raw);
    }
    Memory(Memory &&) noexcept = default;
    Memory &operator=(Memory &&) noexcept = default;
    Memory(const Memory &) = delete;
    Memory &operator=(const Memory &) = delete;
    explicit operator bool() const { return static_cast<bool>(context_); }
    int status() const { return context_ ? 0 : (status_ ? status_ : -EBADF); }
    pid_t pid() const { return fs_pid(context_.get()); }
    ssize_t read(uint64_t address, void *buffer, size_t size) {
        return fs_read(context_.get(), address, buffer, size);
    }
    ssize_t write(uint64_t address, const void *buffer, size_t size) {
        return fs_write(context_.get(), address, buffer, size);
    }
    template<class T> bool read(uint64_t address, T &value) {
        static_assert(std::is_trivially_copyable<T>::value, "Read only plain data");
        T temporary{};
        if (read(address, &temporary, sizeof(T)) != sizeof(T)) return false;
        value = temporary;
        return true;
    }
    template<class T> bool write(uint64_t address, const T &value) {
        static_assert(std::is_trivially_copyable<T>::value, "Write only plain data");
        return write(address, &value, sizeof(T)) == sizeof(T);
    }
    int refresh_modules() { return fs_refresh_modules(context_.get()); }
    int module_count(size_t &count) { return fs_module_count(context_.get(), &count); }
    int module_at(size_t position, fs_module &out) { return fs_module_at(context_.get(), position, &out); }
    int module(const char *name, fs_module &out, uint32_t index = 1, fs_range range = FS_CD) {
        return fs_find_module(context_.get(), name, index, range, &out);
    }
};
}
