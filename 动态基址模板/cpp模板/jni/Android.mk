LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fs
LOCAL_SRC_FILES := ../lib/$(TARGET_ARCH_ABI)/libfs.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := memory
LOCAL_SRC_FILES := ../memory.cpp
LOCAL_CPPFLAGS := -std=c++17 -O2 -fvisibility=hidden
LOCAL_STATIC_LIBRARIES := fs
include $(BUILD_EXECUTABLE)
