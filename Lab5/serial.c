#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "serial.h"
#include "driver/uart.h"

int serial_init(int num)
{
	switch(num) {
	    case UART_NUM_0: //log output
		{
			uart_config_t uart_config = {
				.baud_rate = 115200,
				.data_bits = UART_DATA_8_BITS,
				.parity = UART_PARITY_DISABLE,
				.stop_bits = UART_STOP_BITS_1,
				.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
				.rx_flow_ctrl_thresh = 0,
			};

			uart_param_config(num, &uart_config);
			uart_set_pin(num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
			uart_driver_install(num, 1024, 1024, 0, NULL, 0);
		}
		break;

	    case UART_NUM_1: 
            return -1;
		    break;

	    case UART_NUM_2:
	    	return -1;
		    break;

	    default:
	    	return -1;
	    	break;
	}

	return 0;
}

void serial_write(int num, const unsigned char *buf, int len)
{
	uart_write_bytes(num, (const char*)buf,len);
}

int serial_read(int num, unsigned char *buf, int len, int timeout)
{
	return uart_read_bytes(num, buf, len, timeout);
}

int serial_delete(int uart_num)
{
	uart_driver_delete(uart_num);
	return 0;
}

#endif /* __SERIAL_H__ */