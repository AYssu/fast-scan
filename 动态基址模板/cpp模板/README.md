# libSea3

<div align="center">

![Version](https://img.shields.io/badge/version-3.0-blue)
![NDK](https://img.shields.io/badge/NDK-r27-green)
![Platform](https://img.shields.io/badge/platform-Android-brightgreen)
![Architecture](https://img.shields.io/badge/arch-arm64--v8a%20%7C%20x86__64-orange)

**一个功能强大的 Android 内存操作与网络通信静态库**

[功能特性](#功能特性) • [快速开始](#快速开始) • [Driver 系统](#driver-系统) • [自动构建](#github-actions-自动构建) • [示例代码](#示例代码)

</div>

---

## 📖 简介

libSea3 是一个专为 Android 平台设计的通用静态库，提供了完整的内存读写、网络通信、加密算法等功能。采用模块化 Driver 架构，支持多种内存访问方式，适用于游戏辅助、性能分析、逆向工程等场景。

### 🎯 核心优势

- 🚀 **Driver 架构**：模块化设计，轻松切换不同的内存读写方式
- 🔧 **多平台支持**：同时支持真机 (ARM64) 和模拟器 (x86_64)
- ⚡ **高性能**：优化的内存访问，支持批量操作和指针链
- 🛠️ **易于集成**：只需几行代码即可完成内存操作
- 🤖 **自动构建**：GitHub Actions 自动化编译和发布

---

## ✨ 功能特性

### 内存操作
- ✅ 多种 Driver 支持（syscall、pread64、kernel 等）
- ✅ 指针链自动解析
- ✅ 模块基址获取
- ✅ 批量读写优化
- ✅ 类型安全的模板方法

### 网络通信
- ✅ 自定义 GET/POST 请求
- ✅ 内置 cURL 支持
- ✅ 多种网络验证算法
- ✅ 无需手机环境依赖

### 加密与安全
- ✅ 多种加密算法集成
- ✅ 数据安全传输
- ✅ 验证机制

### 开发辅助
- ✅ 丰富的工具函数
- ✅ 类型安全的接口
- ✅ 详细的错误处理
- ✅ 完善的日志系统

---

## 🚀 快速开始

### 环境要求

- Android NDK r27+
- CMake 3.10+ 或 ndk-build
- Android API Level 21+
- C++17 支持

### 编译项目

```bash
# 克隆仓库
git clone https://github.com/AYssu/libSea3.git
cd libSea3

# 使用 ndk-build 编译
export ANDROID_NDK_HOME=/path/to/ndk
ndk-build

# 编译特定架构
ndk-build APP_ABI=arm64-v8a
ndk-build APP_ABI=x86_64

# 或使用一键构建脚本
./一键构建脚本.sh
```

### 基础使用

```cpp
#include "smemory.h"
#include "driver/driver_syscall.h"

using namespace pointer;

int main() {
    // 1. 获取目标进程 PID
    auto pid = smemory::get_package_pid("com.example.game");
    
    // 2. 初始化 Driver
    static auto driver = new syscall_driver();
    driver->set_pid(pid);
    
    // 3. 设置读写回调
    smemory::set_read<uintptr_t>([&](uintptr_t addr, void *data, size_t size) {
        return driver->read(addr, data, size);
    });
    
    smemory::set_write<uintptr_t>([&](uintptr_t addr, void *data, size_t size) {
        return driver->write(addr, data, size);
    });
    
    // 4. 获取模块基址
    auto base = smemory::get_module_base_str("libil2cpp.so", 1, "Xa");
    
    // 5. 读取数据
    int value = read_int(base + 0x1234);
    std::cout << "读取的值: " << value << std::endl;
    
    // 6. 写入数据
    write_int(base + 0x1234, 999);
    
    return 0;
}
```

---

## 🔌 Driver 系统

libSea3 采用可插拔的 Driver 架构，支持多种内存访问方式：

### 可用的 Driver

| Driver | 文件 | 适用场景 | 性能 |
|--------|------|---------|------|
| **syscall_driver** | `driver/driver_syscall.h` | 真机 ARM64，使用 `process_vm_readv` | ⭐⭐⭐⭐⭐ |
| **pread_driver** | `driver/driver_pread.h` | 模拟器 x86_64，通过 `/proc/pid/mem` | ⭐⭐⭐⭐ |
| **kernel_driver** | `driver/driver_*.h` | 内核驱动方式 | ⭐⭐⭐⭐⭐ |

### Driver 使用示例

```cpp
// 方式 1: 使用 syscall (真机推荐)
#include "driver/driver_syscall.h"
auto driver = new syscall_driver();

// 方式 2: 使用 pread64 (模拟器推荐)
#include "driver/driver_pread.h"
auto driver = new pread_driver();

// 方式 3: 根据平台自动选择
#ifdef __x86_64__
    #include "driver/driver_pread.h"
    auto driver = new pread_driver();
#else
    #include "driver/driver_syscall.h"
    auto driver = new syscall_driver();
#endif
```

### Driver 接口

所有 Driver 都实现了统一的基类接口：

```cpp
class driver_base {
public:
    virtual bool set_pid(pid_t pid) = 0;
    virtual bool read(uintptr_t address, void* buffer, size_t size) = 0;
    virtual bool write(uintptr_t address, void* buffer, size_t size) = 0;
    
    // 模板方法
    template <typename T>
    T read(uintptr_t address);
    
    template <typename T>
    bool write(uintptr_t address, T value);
};
```

---

## 🤖 GitHub Actions 自动构建

项目配置了完整的 CI/CD 流程，支持自动化构建和发布。

### 触发方式

#### 1. 手动触发（推荐）

在 GitHub 仓库页面：
1. 进入 `Actions` 标签
2. 选择 `基址修改在线构建` workflow
3. 点击 `Run workflow`
4. 填写构建参数（所有参数都有默认值）

#### 2. Tag 触发

```bash
# 创建并推送 tag 即可触发构建
git tag v1.0.0
git push origin v1.0.0
```

### 构建参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `package_name` | `com.example.app` | 目标应用包名 |
| `module` | `libil2cpp.so` | 目标模块名称 |
| `memory` | `Xa` | 内存区域标识 |
| `module_index` | `1` | 模块索引 |
| `pointer_chain` | ` ` (空) | 指针链，用 `+` 分隔，如 `0x10+0x20` |
| `modify_mode` | `读取` | 操作类型：读取/修改/读取并修改/修改冻结 |
| `value_type` | `int` | 数据类型：int/float |
| `modify_value` | `999` | 修改的值 |
| `platform` | `真机 64` | 平台：真机 64/模拟器 64 |
| `rw_type` | `syscall` | 读写类型：syscall/prw64 |

### 构建输出

构建完成后会生成：
- `libs/arm64-v8a/memory` - ARM64 可执行文件
- `libs/x86_64/memory` - x86_64 可执行文件

---

## 📚 示例代码

### 指针链读取

```cpp
// 获取模块基址
auto base = smemory::get_module_base_str("libil2cpp.so", 1, "Xa");

// 定义指针链
std::vector<uintptr_t> offsets = {0x10, 0x20, 0x30, 0x40};

// 解析指针链
uintptr_t final_addr = pointer::get_pointer64(base, offsets);

// 读取最终地址的值
int value = read_int(final_addr);
```

### 批量操作

```cpp
// 批量读取
std::vector<int> values(10);
for (int i = 0; i < 10; i++) {
    values[i] = read_int(base + 0x1000 + i * 4);
}

// 批量写入
for (int i = 0; i < 10; i++) {
    write_int(base + 0x1000 + i * 4, i * 100);
}
```

### 类型安全操作

```cpp
// 支持多种数据类型
float health = read_float(addr);
double money = read_double(addr);
int64_t exp = read_long(addr);

write_float(addr, 999.99f);
write_double(addr, 123456.789);
write_long(addr, 999999999LL);
```

---

## 📁 项目结构

```
libSea3/
├── driver/              # Driver 实现
│   ├── driver_base.h    # Driver 基类
│   ├── driver_syscall.h # syscall 实现
│   ├── driver_pread.h   # pread64 实现
│   └── ...
├── include/             # 头文件
│   ├── smemory.h        # 内存操作主接口
│   ├── smmap.h          # 内存映射
│   └── ...
├── jni/                 # NDK 构建配置
│   ├── Android.mk       # ndk-build 配置
│   └── Application.mk   # 应用配置
├── lib/                 # 预编译静态库
│   ├── arm64-v8a/
│   └── x86_64/
├── .github/             # GitHub Actions
│   └── workflows/
│       └── memory.yml   # 自动构建配置
├── main.cpp             # 示例程序
├── memory.cpp           # 内存操作模板
└── README.md
```

---

## 🔧 编译选项

### Android.mk 配置

```makefile
# 优化级别
LOCAL_CFLAGS := -std=c++17 -O3 -s -fvisibility=hidden

# 头文件路径
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/..

# 架构支持
APP_ABI := arm64-v8a x86_64

# 平台版本
APP_PLATFORM := android-21
```

---

## 🐛 常见问题

### Q: 为什么 x86_64 模拟器上 syscall 不工作？
**A:** x86_64 Android 模拟器通常禁用了 `process_vm_readv` 系统调用。请使用 `pread_driver` 替代。

### Q: 如何切换不同的 Driver？
**A:** 修改 include 和 driver 初始化代码：
```cpp
// 从
#include "driver/driver_syscall.h"
auto driver = new syscall_driver();

// 改为
#include "driver/driver_pread.h"
auto driver = new pread_driver();
```

### Q: 编译时出现换行符警告？
**A:** 项目已包含 `.gitattributes` 文件自动处理换行符。如果仍有问题：
```bash
# 转换所有源文件
find . -name "*.cpp" -o -name "*.h" | xargs dos2unix
```

### Q: 如何调试 GitHub Actions 构建？
**A:** 查看构建日志中的 "生成的 memory.cpp" 部分，确认参数替换是否正确。

---

## 🤝 贡献

我们欢迎各种形式的贡献！

### 如何贡献

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

### 贡献指南

- **Bug 报告**：使用 [Issue](https://github.com/AYssu/libSea3/issues) 报告 bug
- **功能建议**：提出新功能需求或改进建议
- **代码规范**：遵循项目的代码风格
- **测试**：确保所有测试通过

---

## 📄 许可证

**本项目采用部分开源许可**

- ✅ **开源部分**：示例代码、头文件接口、Driver 实现
- 🔒 **闭源部分**：核心静态库 (`lib/` 目录下的 `.a` 文件)

### 使用条款

1. 开源部分遵循 MIT 许可证，可自由使用和修改
2. 闭源的核心库仅供学习和个人使用
3. 商业使用需获得授权，请联系作者
4. 禁止逆向工程、反编译或破解核心库

如需商业授权或有其他疑问，请通过邮箱联系：[1773714466@qq.com](mailto:1773714466@qq.com)

---

## 📞 联系方式

- **Email**: [1773714466@qq.com](mailto:1773714466@qq.com)
- **Issues**: [GitHub Issues](https://github.com/AYssu/libSea3/issues)

---

## 🌟 致谢

感谢所有为 libSea3 做出贡献的开发者！

如果这个项目对你有帮助，欢迎给个 ⭐ Star！

---

<div align="center">

Made with ❤️ by AYssu

</div>
