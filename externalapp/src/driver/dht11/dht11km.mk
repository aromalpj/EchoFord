DHT11_SITE = $(BR2_EXTERNAL_ECHOFORD_EXTERNAL_APP_SAMPLE_PATH)/src/driver/dht11
DHT11_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
