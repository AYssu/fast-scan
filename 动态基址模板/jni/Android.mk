LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libSea3
LOCAL_SRC_FILES := ../lib/$(TARGET_ARCH_ABI)/libSea3.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

# 设置模块名称
LOCAL_MODULE := libfscan
LOCAL_STATIC_LIBRARIES := libSea3

# 设置编译器标志
LOCAL_CFLAGS := -std=c++17
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++17 -fvisibility=hidden 

# 设置头文件路径
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

# 设置源文件路径
LOCAL_SRC_FILES := ../memory.cpp \
              ../src/syscall_rv.cpp
    
# 设置链接器标志
LOCAL_LDFLAGS += -llog -landroid -lEGL -lGLESv3 

# 生成可执行文件
include $(BUILD_EXECUTABLE)
