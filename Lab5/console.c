#include "console.h"
#include "serial.h"
#include "wifi.h"
#include "os.h"

/**********************************************************/
static void _help(int argc, char **argv);
static void _wifi_info(int argc, char **argv);
static void _reboot(int argc, char **argv);
static void _show_mem(int argc, char**argv);

static cmd_entry cmd_list[] = \
{
    { "help", "Print help info", _help},
    { "wifi_info", "Show Wi-Fi information", _wifi_info},
    { "reboot", "Reboot", _reboot},
    { "show_mem", "Show system available heap size.", _show_mem},
    //add more
};

static void _help(int argc, char **argv)
{
    for (int i = 0; i < (sizeof(cmd_list)/sizeof(cmd_entry)); i++) {
        LOG_PRINTF("%-30s %s", cmd_list[i].name, cmd_list[i].help ? cmd_list[i].help : "");
	}
}

static void _wifi_info(int argc, char **argv)
{
    wifi_ap_record_t ap_info = {0};
    wifi_config_t cfg = {0};

	if(ESP_OK != esp_wifi_sta_get_ap_info(&ap_info)) {
         LOG_PRINTF("Get Wi-Fi info ERROR");
		return;
    }

    LOG_PRINTF("BSSID: "MACSTR"", MAC2STR(ap_info.bssid));
    LOG_PRINTF("SSID: %s", ap_info.ssid);
    LOG_PRINTF("Channel: %u", ap_info.primary);
    LOG_PRINTF("RSSI: %d", ap_info.rssi);

    esp_wifi_get_config(ESP_IF_WIFI_STA, &cfg);
    LOG_PRINTF("Password: %s", cfg.sta.password);
}

static void _reboot(int argc, char **argv)
{
    LOG_PRINTF("Reboot in 1s.");
    vTaskDelay(MS2TICK(1000));
    esp_restart();
}

static void _show_mem(int argc, char **argv)
{
    LOG_PRINTF("Free heap size: %u bytes", esp_get_free_heap_size());
}

static int _console_recv(console_t* console)
{
	uint8_t c = 0;
	uint32_t bytes = 0;
	
	while(1) {
        int ret = serial_read(console->uart_num, &c, 1, WAIT_FOREVER);
		if (1 == ret) {
            if ('\r' == c) {
                break;
            }else {
                //visiable? cut empty head
                if(c > 31 && c < 127 && !(bytes == 0 && c == ' ')) {
                    console->cmd_buf[bytes++] = c;
                }
                if (bytes >= CMD_SIZE_MAX) {
                    LOG_ERROR("Invalid cmd: overlength-strings");
                    return -1;
                }
            }
		} else if(-1 == ret) {
            LOG_ERROR("Invalid cmd: UART read ERROR");
        }
    }

    console->cmd_buf[bytes] = '\0';

    return bytes;
}

static bool _console_verify_pwd(const char* buf)
{
    if(NULL == buf) {
        LOG_ERROR("Invalid PWD: NULL");
        return false;
    }

    if(0 != strcmp(buf, CONSOLE_PWD)) {
        LOG_ERROR("Invalid PWD: Wrong input");
        return false;
    }

    return true;
}

static void _console_execute(const char* buf)
{
    //find cmd
    int i = 0;

    if((!buf) || (strlen(buf) == 0)) {
        LOG_ERROR("cmd is NULL!");
        return;
    }

    while (i < (sizeof(cmd_list)/sizeof(cmd_entry))) {
        if (cmd_list[i].name != NULL && strcmp(cmd_list[i].name, buf) == 0) {
            //enter
            LOG_PRINTF("%s>>", cmd_list[i].name);
            cmd_list[i].function(0, NULL);
            LOG_PRINTF("\r\n");
            return;
        }
        i++;
    }

    LOG_ERROR("cmd not found!");

    return;
}

static void console_main(void* arg)
{  
    console_t *console = (console_t*)arg;
    
    while(CONSOLE_CLOSED != console->state) {
        switch(console->state){
            //requiring password
            case CONSOLE_PWD_REQUIRED:
                _console_recv(console);
                if(true == _console_verify_pwd(console->cmd_buf)) {
                    LOG_PRINTF("\r\n");
                    LOG_PRINTF("-------------------------------------------------------------------");
                    LOG_PRINTF("          Authentication Passed, Debug Console Activated");
                    LOG_PRINTF("-------------------------------------------------------------------");
                    LOG_PRINTF("\r\n");
                    console->state = CONSOLE_ACTIVE;
                }else {
                    //cooldown?
                }

                break;

            //input mode
            case CONSOLE_ACTIVE:
                _console_recv(console);
                _console_execute(console->cmd_buf);
                break;

            default:
                break;
        }
    }

    console->thread = NULL;
    vTaskDelete(NULL);
}

void console_deinit(console_t* console)
{
    console->state = CONSOLE_CLOSED;
    console->echo = false;
    console->uart_num = -1;
    memset(console->cmd_buf, 0, CMD_SIZE_MAX);
    vSemaphoreDelete(&console->cmd_buf_mutex);
}

int console_init(console_t *console, int uart_num)
{
    if(console != NULL) {
        memset(console, 0, sizeof(console_t));
    }
    else {
        LOG_ERROR("Console is NULL, init failed!");
        goto error_exit;
    }

    console->state = CONSOLE_PWD_REQUIRED;
    console->echo = false;
    console->uart_num = uart_num;
    memset(console->cmd_buf, 0, CMD_SIZE_MAX);
    console->cmd_buf_mutex = xSemaphoreCreateMutex();
    if(!console->cmd_buf_mutex)
    {
        LOG_ERROR("console mutex create failed!");
        goto error_exit;
    }

    int ret = xTaskCreate((TaskFunction_t)console_main, "Console Task", 2048, (void*)console, PRIORITY_DEFAULT, &console->thread);
    if(ret != pdPASS) {
        LOG_ERROR("mcmd_main create failed\r\n");
        goto error_exit;
    }
	return 0;

error_exit:
    console_deinit(console);
	return -1;
}
