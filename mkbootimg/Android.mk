
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := mkbootimg.c
LOCAL_STATIC_LIBRARIES := libmincrypt

LOCAL_MODULE := mkbootimg

include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := unpackbootimg.c
LOCAL_MODULE := unpackbootimg
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mkbootimg.c
LOCAL_STATIC_LIBRARIES := libmincrypt libcutils libc
LOCAL_MODULE := mkbootimg
LOCAL_MODULE_CLASS := UTILITY_EXECUTABLES
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/utilities/
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := unpackbootimg.c
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_MODULE := unpackbootimg
LOCAL_MODULE_CLASS := UTILITY_EXECUTABLES
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/utilities/
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))
