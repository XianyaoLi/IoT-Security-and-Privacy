#include "host.h"
#include "log.h"
#include "esp_sntp.h"

void init_host(host_t* host)
{
  wifi_init_sta(host->wifi_creds.Wifi_SSID, host->wifi_creds.Wifi_Pass);

  // Configure time
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  // Setup I2C for OLED and Env Sensor
  if(0 != i2c_init(I2C_NUM_0))
  {
      LOG_PRINTF("Failed to Initialize I2C for screen");
  }

  // Setup Env Sensor
  htu21_init();
  host->htu21.msg_queue = xQueueCreate(1, sizeof(htu21_data_t));
  if(host->htu21.msg_queue == NULL)
  {
    LOG_ERROR("Failed to create Queue");
  }
  xTaskCreate((TaskFunction_t)htu21_main, "Sensor Task", 2048,
    (void*)&(host->htu21), PRIORITY_DEFAULT, &(host->htu21.thread));

  // Setup the OLED Screen
  host->ssd1306.state = ENV_INFO;
  xTaskCreate((TaskFunction_t)ssd1306_main, "OLED_thread", 2048, 
    (void*)host, PRIORITY_DEFAULT, &(host->ssd1306.thread));
  
  // Setup the console
  host->console.echo = true;
  host->console.uart_num = UART_NUM_0;
  serial_init(host->console.uart_num);
  console_init(&(host->console), host->console.uart_num);
}
