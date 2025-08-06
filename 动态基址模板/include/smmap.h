#pragma once
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace smmap {

    /**
     * 创建mmap 映射
     * @param file_name
     * @param read_only
     * @param msize
     * @return 指针
     */
    void *build_mmap_void(const char *file_name, bool read_only, size_t msize);

}// namespace smmap
