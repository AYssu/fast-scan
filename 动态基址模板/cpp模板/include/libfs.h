#ifndef LIBFS_H
#define LIBFS_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(__GNUC__)
#define FS_API __attribute__((visibility("default")))
#else
#define FS_API
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct fs_context fs_context;
typedef enum fs_backend { FS_PROCESS_VM = 0, FS_PROC_MEM = 1 } fs_backend;
typedef enum fs_range {
    FS_CB = 8, FS_CD = 16, FS_XA = 16384, FS_CDRO = 299703, FS_CDX = 299704
} fs_range;

typedef struct fs_module {
    uint64_t start;
    uint64_t end;
    uint64_t file_offset;
    uint32_t index; /* 1-based, separately numbered for each name + range. */
    int32_t range;
    uint32_t protection; /* read=1, write=2, execute=4 */
    char name[256]; /* BSS retains the FastScan suffix, e.g. libgame.so:bss. */
    char path[256];
} fs_module;

/* Callbacks return bytes transferred, or negative errno (not boolean).
 * read is required; write and destroy are optional. The user object is owned
 * by the context ONLY after successful fs_open_callbacks. Never throw across
 * these callbacks. Calls on one context are serialized. */
typedef struct fs_callbacks {
    void *user;
    ssize_t (*read)(void *user, pid_t pid, uint64_t address, void *buffer, size_t size);
    ssize_t (*write)(void *user, pid_t pid, uint64_t address, const void *buffer, size_t size);
    void (*destroy)(void *user);
} fs_callbacks;

/* Status functions return 0 or negative errno. No implicit su, fallback reader,
 * target switching, stdout output, or network operation occurs. */
FS_API const char *fs_version(void);
FS_API int fs_open(pid_t pid, fs_backend backend, fs_context **out);
FS_API int fs_open_callbacks(pid_t pid, const fs_callbacks *callbacks, fs_context **out);
FS_API void fs_close(fs_context *context);
FS_API pid_t fs_pid(const fs_context *context);
/* Read/write complete short transfers when possible. On a later failure they
 * return the transferred prefix; failure before any transfer is negative errno.
 * A failed/partial write may already have modified target memory. */
FS_API ssize_t fs_read(fs_context *context, uint64_t address, void *buffer, size_t size);
FS_API ssize_t fs_write(fs_context *context, uint64_t address, const void *buffer, size_t size);
/* Refresh is transactional. On failure the last successful snapshot remains.
 * Enumeration/query never refresh implicitly. Refresh after dlopen/dlclose. */
FS_API int fs_refresh_modules(fs_context *context);
FS_API int fs_module_count(fs_context *context, size_t *count);
FS_API int fs_module_at(fs_context *context, size_t position, fs_module *out);
FS_API int fs_find_module(fs_context *context, const char *name, uint32_t index,
                          fs_range range, fs_module *out);
FS_API const char *fs_range_label(int range);

#ifdef __cplusplus
}
#endif
#endif
