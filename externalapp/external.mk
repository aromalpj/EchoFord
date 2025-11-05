include $(sort $(wildcard $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_PATH)/package/*/*.mk))
include $(sort $(wildcard $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_PATH)/src/*/*/*.mk))

BR2_ROOTFS_OVERLAY += $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_PATH)/rootfs-overlay
