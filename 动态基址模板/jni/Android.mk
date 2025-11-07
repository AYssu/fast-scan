LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libSea3
LOCAL_SRC_FILES := ../lib/$(TARGET_ARCH_ABI)/libSea3.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

# 设置模块名称
LOCAL_MODULE := memory
LOCAL_STATIC_LIBRARIES := libSea3

# 设置编译器标志（全局）
# 注意：使用 -O3 而非 -Ofast，避免与 thread_local + syscall 的已知优化 bug
LOCAL_CFLAGS := -std=c++17 -O3 -s -fvisibility=hidden -Wno-unused-command-line-argument
LOCAL_CPPFLAGS := -std=c++17 -O3 -s -fvisibility=hidden -Wno-unused-command-line-argument

# 设置头文件路径（包含 driver 目录）
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/..

# 设置源文件路径
LOCAL_SRC_FILES := ../memory.cpp
    
# 设置链接器标志
LOCAL_LDFLAGS += -llog -landroid -lEGL -lGLESv3 

# 生成可执行文件
include $(BUILD_EXECUTABLE)
