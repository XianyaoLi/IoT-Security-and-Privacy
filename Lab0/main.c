#include "labs.h"
#include "host.h"
#include "os_func.h"
#include <stdio.h>
#include "cryptoauthlib.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "driver/i2c.h"
#include "crypto_settings.h"
#include "freertos/task.h"
#include "csr_def.h"
#include "led.h"

/**
 * Main 
 */

void app_main(void)
{
    host_config_t host_config = {
        .encrypt_type = ENCRYPT_TYPE_AES,
        .console_uart_num = UART_NUM_0,
        .wifi_if = {
            .wifi_init = labs_wifi_init,
            .wifi_disconnect = NULL
        },
        .plt_if = {
            .drivers_init = platform_init,
            .time_init = utc_set
        }
    };

    host_handle_t host_handle = host_create(&host_config);

    ms_sleep(1000);
	
    led_init();
	
    uint8_t count = 0;
    while(1)
    {
		led_display_num(count++);
        vTaskDelay(MS2TICK(500));
    }
}
