LOCAL_PATH:= $(call my-dir)

SRC := arsc_parser.cpp

C_INCLUDES += \
	frameworks/base/include \
    system/core/base/include


include $(CLEAR_VARS)

LOCAL_MODULE := arsc_parser
LOCAL_SRC_FILES := $(SRC)
LOCAL_C_INCLUDES += $(C_INCLUDES)
LOCAL_STATIC_LIBRARIES := libutils liblog

include $(BUILD_HOST_EXECUTABLE)



