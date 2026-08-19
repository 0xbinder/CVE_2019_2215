LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := poc
LOCAL_SRC_FILES := src/binder_uaf.c src/binder.c src/cpu_affinity.c src/event_poll.c src/kernel_rw.c src/log.c src/corrupt_address_limit.c src/privilege_escalation.c src/main.c
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := trigger
LOCAL_SRC_FILES := trigger/trigger.c
include $(BUILD_EXECUTABLE)