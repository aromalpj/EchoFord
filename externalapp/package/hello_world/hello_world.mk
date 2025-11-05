HELLO_WORLD_SITE = $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_PATH)/package/hello_world
HELLO_WORLD_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
