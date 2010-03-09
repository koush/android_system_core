
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := mkbootimg.c
LOCAL_STATIC_LIBRARIES := libmincrypt

LOCAL_MODULE := mkbootimg

include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := unpackbootimg.c
LOCAL_STATIC_LIBRARIES := libmincrypt

LOCAL_MODULE := unpackbootimg

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,droid,$(LOCAL_BUILT_MODULE))
