I2C_CLIENT_SITE = $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_SAMPLE_PATH)/src/driver/i2cappClient
I2C_CLIENT_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
