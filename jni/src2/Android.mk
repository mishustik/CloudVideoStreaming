LOCAL_PATH := $(call my-dir)

#declare the prebuilt library
include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg-prebuilt
LOCAL_SRC_FILES := ../ffmpeg/libffmpeg.so
LOCAL_EXPORT_C_INCLUDES := ../ffmpeg
LOCAL_EXPORT_LDLIBS := ../ffmpeg/libffmpeg.so
LOCAL_PRELINK_MODULE := true
LOCAL_CFLAGS := -g -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon

include $(PREBUILT_SHARED_LIBRARY)

#the andzop library
include $(CLEAR_VARS)
LOCAL_ALLOW_UNDEFINED_SYMBOLS=false
LOCAL_MODULE := mylib
LOCAL_SRC_FILES := mylib.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../ffmpeg \
LOCAL_SHARED_LIBRARY := ffmpeg-prebuilt
LOCAL_LDLIBS    := $(LOCAL_PATH)/../ffmpeg/libffmpeg.so -llog -ldl -ljnigraphics -lz -lm
LOCAL_CFLAGS := -g -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon

include $(BUILD_SHARED_LIBRARY)