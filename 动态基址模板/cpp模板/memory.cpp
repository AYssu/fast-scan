#include "libfs.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s PID [integer-address, e.g. 0x1234]\n", argv[0]); return 2;
    }
    char *end = nullptr;
    long pid = strtol(argv[1], &end, 10);
    if (!end || *end || pid <= 0 || pid > INT32_MAX) return 2;
    libfs::Memory memory(static_cast<pid_t>(pid));
    if (!memory) { fprintf(stderr, "open: %s\n", strerror(-memory.status())); return 1; }
    int status = memory.refresh_modules();
    if (status) { fprintf(stderr, "modules: %s\n", strerror(-status)); return 1; }
    size_t count = 0;
    if (memory.module_count(count)) return 1;
    for (size_t i = 0; i < count; ++i) {
        fs_module m{};
        if (memory.module_at(i, m)) return 1;
        printf("%s[%u][%s] 0x%llx-0x%llx\n", m.name, m.index, fs_range_label(m.range),
               static_cast<unsigned long long>(m.start), static_cast<unsigned long long>(m.end));
    }
    if (argc == 3) {
        errno = 0;
        uint64_t address = strtoull(argv[2], &end, 0);
        if (errno || !end || *end || argv[2][0] == '-') return 2;
        int32_t value = 0;
        ssize_t n = memory.read(address, &value, sizeof(value));
        if (n != sizeof(value)) { fprintf(stderr, "read: %zd bytes/status\n", n); return 1; }
        printf("int32[0x%llx] = %d\n", static_cast<unsigned long long>(address), value);
    }
}
