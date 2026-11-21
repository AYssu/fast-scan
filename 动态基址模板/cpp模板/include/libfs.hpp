#pragma once
#include "libfs.h"
#include <cerrno>
#include <memory>
#include <type_traits>
#include <dirent.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <system_error>
#include <cstring>

namespace libfs {

// Exact cmdline match: a package does not silently select its :service process.
// Multiple visible matches require an explicit PID instead of guessing a clone.
inline pid_t find_pid(const char *process_name) {
    if (!process_name || !*process_name) return -EINVAL;
    struct CloseDir { void operator()(DIR *p) const { closedir(p); } };
    std::unique_ptr<DIR, CloseDir> proc(opendir("/proc"));
    if (!proc) return -errno;
    pid_t found = -ESRCH;
    while (auto *entry = readdir(proc.get())) {
        char *end = nullptr;
        errno = 0;
        long number = strtol(entry->d_name, &end, 10);
        if (errno || !end || *end || number <= 0 || number > INT32_MAX) continue;
        std::ifstream file(std::string("/proc/") + entry->d_name + "/cmdline", std::ios::binary);
        std::string name;
        std::getline(file, name, '\0');
        if (name != process_name) continue;
        if (found > 0) return -EEXIST;
        found = static_cast<pid_t>(number);
    }
    return found;
}


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
    // Same convention as Sea3 get_pointer64: dereference all but the last offset.
    // Failure leaves result unchanged; pointer width is the TARGET width.
    template<class Pointer> bool pointer(uint64_t base, std::initializer_list<int64_t> offsets,
                                        uint64_t &result) {
        static_assert(std::is_same<Pointer, uint32_t>::value ||
                      std::is_same<Pointer, uint64_t>::value, "Use a 32 or 64 bit pointer");
        if (offsets.size() == 0) return false;
        uint64_t address = base;
        size_t index = 0;
        constexpr uint64_t maximum = std::numeric_limits<Pointer>::max();
        for (int64_t offset : offsets) {
            if (address > maximum) return false;
            const uint64_t magnitude = offset < 0 ? uint64_t(-(offset + 1)) + 1 : uint64_t(offset);
            if (offset < 0) {
                if (magnitude > address) return false;
                address -= magnitude;
            } else {
                if (magnitude > maximum - address) return false;
                address += magnitude;
            }
            if (++index < offsets.size()) {
                Pointer value{};
                if (!read(address, value) || !value) return false;
                address = value;
            }
        }
        result = address;
        return true;
    }
    bool pointer64(uint64_t base, std::initializer_list<int64_t> offsets, uint64_t &result) {
        return pointer<uint64_t>(base, offsets, result);
    }
    bool pointer32(uint64_t base, std::initializer_list<int64_t> offsets, uint64_t &result) {
        return pointer<uint32_t>(base, offsets, result);
    }
    // Value-returning helpers throw on failed/partial reads instead of returning fake zeroes.
    template<class T> T read_value(uint64_t address) {
        static_assert(std::is_trivially_copyable<T>::value, "Read only plain data");
        T value{};
        const ssize_t count = read(address, &value, sizeof(value));
        if (count != sizeof(value))
            throw std::system_error(count < 0 ? int(-count) : EIO, std::generic_category(), "read memory");
        return value;
    }
    uint64_t lsp64(uint64_t address) { return read_value<uint64_t>(address); }
    uint32_t lsp32(uint64_t address) { return read_value<uint32_t>(address); }
    int32_t read_int(uint64_t address) { return read_value<int32_t>(address); }
    float read_float(uint64_t address) { return read_value<float>(address); }
    double read_double(uint64_t address) { return read_value<double>(address); }
    bool write_int(uint64_t address, int32_t value) { return write(address, value); }
    bool write_float(uint64_t address, float value) { return write(address, value); }
    bool write_double(uint64_t address, double value) { return write(address, value); }
    uint64_t read_pointer(uint64_t base, std::initializer_list<int64_t> offsets) {
        uint64_t result{};
        if (!pointer64(base, offsets, result))
            throw std::system_error(EFAULT, std::generic_category(), "read pointer chain (64 bit)");
        return result;
    }
    uint64_t read_pointer32(uint64_t base, std::initializer_list<int64_t> offsets) {
        uint64_t result{};
        if (!pointer32(base, offsets, result))
            throw std::system_error(EFAULT, std::generic_category(), "read pointer chain (32 bit)");
        return result;
    }
    uint64_t get_module_base_str(const char *name, uint32_t index = 1, const char *range = "Cd") {
        fs_range selected{};
        if (range && std::strcmp(range, "Cd") == 0) selected = FS_CD;
        else if (range && std::strcmp(range, "Cb") == 0) selected = FS_CB;
        else if (range && std::strcmp(range, "Xa") == 0) selected = FS_XA;
        else if (range && std::strcmp(range, "Cdro") == 0) selected = FS_CDRO;
        else if (range && std::strcmp(range, "Cdx") == 0) selected = FS_CDX;
        else throw std::invalid_argument("module range must be Cd, Cb, Xa, Cdro or Cdx");
        fs_module item{};
        const int error = module(name, item, index, selected);
        if (error) throw std::system_error(-error, std::generic_category(), "find module");
        return item.start;
    }
    int refresh_modules() { return fs_refresh_modules(context_.get()); }
    int module_count(size_t &count) { return fs_module_count(context_.get(), &count); }
    int module_at(size_t position, fs_module &out) { return fs_module_at(context_.get(), position, &out); }
    int module(const char *name, fs_module &out, uint32_t index = 1, fs_range range = FS_CD) {
        return fs_find_module(context_.get(), name, index, range, &out);
    }
};
}
