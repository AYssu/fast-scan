//
// Created by 阿夜 on 4/1/25.
//

#include "rv.h"

#include <chrono>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;

pid_t target_pid = -1;
class c_driver {
private:
    int fd;

    typedef struct _COPY_MEMORY {
        pid_t pid;
        uintptr_t address;
        void* buffer;
        size_t size;
    } COPY_MEMORY, *PCOPY_MEMORY;

    typedef struct _MODULE_BASE {
        pid_t pid;
        char* name;
        uintptr_t base;
    } MODULE_BASE, *PMODULE_BASE;

    enum OPERATIONS {
        OP_INIT_KEY = 0x800,
        OP_READ_MEM = 0x801,
        OP_WRITE_MEM = 0x802,
        OP_MODULE_BASE = 0x803,
    };

    bool StrAlpha(const char* str)
    {
        for (int i = 0; i < 6; i++) {
            if (!isalpha(str[i])) {
                return false;
            }
        }
        return true;
    }
    int start_driver()
    {
        DIR* dir;
        struct dirent* ptr;
        struct stat info;
        dir = opendir("/proc");
        ssize_t len;
        char path[1024];
        char buffer[1024];
        char fd_path[1024];
        char fd_buffer[1024];
        char dev_path[1024];
        char data_path[1024];
        int ID;
        int PPID;

        while ((ptr = readdir(dir)) != NULL) {
            if (ptr->d_type == DT_DIR) {
                sprintf(buffer, "/proc/%d/exe", atoi(ptr->d_name));
                len = readlink(buffer, path, sizeof(path) - 1);
                if (len != -1)
                    path[len] = '\0';
                char* stres = strstr(path, "(deleted)");
                if (stres != NULL) {
                    sscanf(path, "/data/%s", data_path);
                    if (StrAlpha(data_path)) {
                        sscanf(buffer, "/proc/%d/exe", &PPID);

                        for (int i = 3; i < 5; i++) {
                            sprintf(fd_path, "/proc/%d/fd/%d", PPID, i);
                            readlink(fd_path, fd_buffer, sizeof(fd_buffer) - 1);
                            char* stress = strstr(fd_buffer, "(deleted)");
                            if (stress != NULL) {
                                int fd_file = open(fd_path, O_RDONLY);
                                if (fd_file == -1) {
                                    perror("open");
                                    close(fd_file);
                                    return EXIT_FAILURE;
                                }
                                if (fstat(fd_file, &info) == -1) {
                                    perror("fstat");
                                    close(fd_file);
                                    return 0;
                                }
                                sscanf(fd_buffer, "%s (deleted)", dev_path);

                                if (access(dev_path, F_OK) == 0) { // 检查文件是否存在
                                    ID = open(dev_path, O_RDWR);
                                    if (ID != -1) {

                                        if (unlink(dev_path) == 0) {
                                            return ID;
                                        }
                                    }
                                } else { // 驱动已经隐藏

                                    mode_t mode = S_IFCHR | 0666; // 创建一个命名管道，权限为读写
                                    dev_t dev = makedev(major(info.st_rdev), minor(info.st_rdev));
                                    if (mknod(dev_path, mode, dev) != -1) {
                                    }
                                    ID = open(dev_path, O_RDWR);
                                    if (ID != -1) {

                                        if (unlink(dev_path) != -1) {

                                            return ID;
                                        }
                                    }
                                }
                                close(fd_file);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
        closedir(dir);
        printf("[+] 未识别到驱动程序\n");
        return -1;
    }

public:
    c_driver()
    {
        fd = start_driver();
        if (fd <= 0) {
            printf("[-] 驱动异常或不存在,请安装QX11.4\n");
            exit(0);
        } else {
            printf("[-] 驱动加载成功\n");
        }
    }

    ~c_driver()
    {
        if (fd > 0) {
            printf("[-] 驱动关闭成功!\n");
            close(fd);
        }
    }


    bool read(uintptr_t address, void* buffer, size_t size)
    {
        COPY_MEMORY cm;
        cm.pid = target_pid;
        cm.address = address;
        cm.buffer = buffer;
        cm.size = size;
        if (ioctl(fd, OP_READ_MEM, &cm) != 0) {
            return false;
        }
        return true;
    }

    bool write(uintptr_t address, void* buffer, size_t size)
    {
        COPY_MEMORY cm;
        cm.pid = target_pid;
        cm.address = address;
        cm.buffer = buffer;
        cm.size = size;
        if (ioctl(fd, OP_WRITE_MEM, &cm) != 0) {
            return false;
        }
        return true;
    }

    template <typename T>
    T read(uintptr_t address)
    {
        T res;
        if (this->read(address, &res, sizeof(T)))
            return res;
        return {};
    }

    template <typename T>
    bool write(uintptr_t address, T value)
    {
        return this->write(address, &value, sizeof(T));
    }

    uintptr_t get_module_base(const char* name)
    {
        MODULE_BASE mb;
        char buf[0x100];
        strcpy(buf, name);
        mb.pid = target_pid;
        mb.name = buf;
        if (ioctl(fd, OP_MODULE_BASE, &mb) != 0) {
            return 0;
        }
        return mb.base;
    }
};

static c_driver* driver = new c_driver();

long smemory_readv::readv(uintptr_t addr, void *data, size_t size) {

    return driver->read(addr, data, size);
}

long smemory_readv::writev(uintptr_t addr, void *data, size_t size) {

    return driver->write(addr, data, size);
}