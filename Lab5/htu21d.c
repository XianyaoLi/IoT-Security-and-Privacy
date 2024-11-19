/**
 * File Name: 	htu21d.c
 * Description: Source file for the htu21d temperature and humidity sensor driver.
 */

/***************************************************************************************************/
/* Include Files */
/***************************************************************************************************/
#include "htu21d.h"
#include "driver/i2c.h"
/***************************************************************************************************/
/* Private Data Types */
/***************************************************************************************************/

/***************************************************************************************************/
/* Private Variables(static) */
/***************************************************************************************************/

/***************************************************************************************************/
/* Public Variable Definitions */
/***************************************************************************************************/
uint32_t htu21_temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_14b_RH_12b;
uint32_t htu21_humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_14b_RH_12b;
htu21_i2c_master_mode_t i2c_master_mode;
/***************************************************************************************************/
/* Private Function Prototypes(static) */
/***************************************************************************************************/
static htu21_status_t htu21_write_command(uint8_t);
static htu21_status_t htu21_write_command_no_stop(uint8_t);
/**
 * @brief Reads the HTU21 user register.
 *
 * @param[out] uint8_t* : Storage of user register value
 *
 * @return htu21_status : status of HTU21
 *       - htu21_status_ok : I2C transfer completed successfully
 *       - htu21_status_i2c_transfer_error : Problem with i2c transfer
 *       - htu21_status_no_i2c_acknowledge : I2C did not acknowledge
 */
static htu21_status_t htu21_read_user_register(uint8_t *);
static htu21_status_t htu21_write_user_register(uint8_t );
static htu21_status_t htu21_temperature_conversion_and_read_adc( uint16_t *);
static htu21_status_t htu21_humidity_conversion_and_read_adc( uint16_t *);
static htu21_status_t htu21_crc_check( uint16_t, uint8_t);

/***************************************************************************************************/
/* Public Function Definitions */
/***************************************************************************************************/
void htu21_init(void)
{
    i2c_master_mode = htu21_i2c_no_hold;
}
bool htu21_is_connected(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    uint8_t data = 0;
    i2c_master_write(cmd, &data, sizeof(data), true);
    i2c_master_stop(cmd);
    esp_err_t status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
    if(status != ESP_OK)
    {
        return false;
    }
    return true;
}
htu21_status_t htu21_reset(void)
{
	htu21_status_t status;
	
	status = htu21_write_command(HTU21_RESET_COMMAND);
	if( status != htu21_status_ok )
		return status;
	
	htu21_temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_14b_RH_12b;
	htu21_humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_14b_RH_12b;
	
	return htu21_status_ok;
}
void htu21_set_i2c_master_mode(htu21_i2c_master_mode_t mode)
{
	i2c_master_mode = mode;
	return;
}
htu21_status_t htu21_read_serial_number(uint64_t* serial_number)
{
	htu21_status_t status;
	esp_err_t i2c_status;
	uint8_t cmd_data[2];
	uint8_t rcv_data[14];
	uint8_t i;

	// Read the first 8 bytes
	cmd_data[0] = (HTU21_READ_SERIAL_FIRST_8BYTES_COMMAND>>8)&0xFF;
	cmd_data[1] = HTU21_READ_SERIAL_FIRST_8BYTES_COMMAND&0xFF;
		
	/* Do the transfer */
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, cmd_data, 2, true);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read(cmd, rcv_data, 8, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);


	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;

	// Read the last 6 bytes
	cmd_data[0] = (HTU21_READ_SERIAL_LAST_6BYTES_COMMAND>>8)&0xFF;
	cmd_data[1] = HTU21_READ_SERIAL_LAST_6BYTES_COMMAND&0xFF;

	/* Do the transfer */
	cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, cmd_data, 2, true);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read(cmd, &rcv_data[8], 6, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;
	
	for( i=0 ; i<8 ; i+=2 ) {
		status = htu21_crc_check(rcv_data[i],rcv_data[i+1]);
		if( status != htu21_status_ok )
			return status;
	}
	for( i=8 ; i<14 ; i+=3 ) {
		status = htu21_crc_check( ((rcv_data[i]<<8)|(rcv_data[i+1])),rcv_data[i+2] );
		if( status != htu21_status_ok )
			return status;
	}
	
	*serial_number =	((uint64_t)rcv_data[0]<<56) | ((uint64_t)rcv_data[2]<<48) | ((uint64_t)rcv_data[4]<<40) | ((uint64_t)rcv_data[6]<<32)
					|	((uint64_t)rcv_data[8]<<24) | ((uint64_t)rcv_data[9]<<16) | ((uint64_t)rcv_data[11]<<8) | ((uint64_t)rcv_data[12]<<0);
	
	return htu21_status_ok;
	
}

htu21_status_t htu21_set_resolution(htu21_resolution_t res)
{
	htu21_status_t status;
	uint8_t reg_value, tmp=0;
	uint32_t temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_14b_RH_12b;
	uint32_t humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_14b_RH_12b;
	
	if( res == htu21_resolution_t_14b_rh_12b) {
		tmp = HTU21_USER_REG_RESOLUTION_T_14b_RH_12b;
		temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_14b_RH_12b;
		humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_14b_RH_12b;
	}
	else if( res == htu21_resolution_t_13b_rh_10b) {
		tmp = HTU21_USER_REG_RESOLUTION_T_13b_RH_10b;
		temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_13b_RH_10b;
		humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_13b_RH_10b;
	}
	else if( res == htu21_resolution_t_12b_rh_8b) {
		tmp = HTU21_USER_REG_RESOLUTION_T_12b_RH_8b;
		temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_12b_RH_8b;
		humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_12b_RH_8b;
	}
	else if( res == htu21_resolution_t_11b_rh_11b) {
		tmp = HTU21_USER_REG_RESOLUTION_T_11b_RH_11b;
		temperature_conversion_time = HTU21_TEMPERATURE_CONVERSION_TIME_T_11b_RH_11b;
		humidity_conversion_time = HTU21_HUMIDITY_CONVERSION_TIME_T_11b_RH_11b;
	}
		
	status = htu21_read_user_register(&reg_value);
	if( status != htu21_status_ok )
		return status;
	
	// Clear the resolution bits
	reg_value &= ~HTU21_USER_REG_RESOLUTION_MASK;
	reg_value |= tmp & HTU21_USER_REG_RESOLUTION_MASK;
	
	htu21_temperature_conversion_time = temperature_conversion_time;
	htu21_humidity_conversion_time = humidity_conversion_time;
	
	status = htu21_write_user_register(reg_value);
	
	return status;
}

htu21_status_t htu21_read_temperature_and_relative_humidity(float* temperature, float* humidity)
{
	htu21_status_t status;
	uint16_t adc = 0;
	
	status = htu21_temperature_conversion_and_read_adc(&adc);
	if( status != htu21_status_ok)
		return status;
	
	// Perform conversion function
	*temperature = (float)adc * TEMPERATURE_COEFF_MUL / (1UL<<16) + TEMPERATURE_COEFF_ADD;

	status = htu21_humidity_conversion_and_read_adc( &adc);
	if( status != htu21_status_ok)
		return status;
	
	// Perform conversion function
	*humidity = (float)adc * HUMIDITY_COEFF_MUL / (1UL<<16) + HUMIDITY_COEFF_ADD;
	
	return status;
}
float htu21_compute_compensated_humidity(float temperature,float relative_humidity)
{
	return ( relative_humidity + (25 - temperature) * HTU21_TEMPERATURE_COEFFICIENT);
}
float htu21_compute_dew_point(float temperature,float relative_humidity)
{
	double partial_pressure;
	double dew_point;
	
	// Missing power of 10
	partial_pressure = pow( 10, HTU21_CONSTANT_A - HTU21_CONSTANT_B / (temperature + HTU21_CONSTANT_C) );
	
	dew_point =  - HTU21_CONSTANT_B / (log10( relative_humidity * partial_pressure / 100) - HTU21_CONSTANT_A) - HTU21_CONSTANT_C;
	
	return (float)dew_point;
}
/***************************************************************************************************/
/* Private Function Definitions */
/***************************************************************************************************/
htu21_status_t htu21_write_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, &command, sizeof(command), true);
    i2c_master_stop(cmd);
    esp_err_t i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;
	
	return htu21_status_ok;
}
htu21_status_t htu21_write_command_no_stop(uint8_t command)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, &command, sizeof(command), true);
    esp_err_t i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;
	
	return htu21_status_ok;
}
htu21_status_t htu21_crc_check(uint16_t value, uint8_t crc)
{
	uint32_t polynom = 0x988000; // x^8 + x^5 + x^4 + 1
	uint32_t msb     = 0x800000;
	uint32_t mask    = 0xFF8000;
	uint32_t result  = (uint32_t)value<<8; // Pad with zeros as specified in spec
	
	while( msb != 0x80 ) {
		
		// Check if msb of current value is 1 and apply XOR mask
		if( result & msb )
			result = ((result ^ polynom) & mask) | ( result & ~mask);
			
		// Shift by one
		msb >>= 1;
		mask >>= 1;
		polynom >>=1;
	}
	if( result == crc )
		return 	htu21_status_ok;
	else
		return htu21_status_crc_error;
}
htu21_status_t htu21_read_user_register(uint8_t *value)
{
	htu21_status_t status;
	esp_err_t i2c_status;
	// Send the Read Register Command
	status = htu21_write_command(HTU21_READ_USER_REG_COMMAND);

	if( status != htu21_status_ok )
		return status;
	uint8_t buff[1];
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read(cmd, buff, 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;

	*value = buff[0];
	
	return htu21_status_ok;
}
htu21_status_t htu21_write_user_register(uint8_t value)
{
	htu21_status_t status;
	esp_err_t i2c_status;
	uint8_t reg;
	uint8_t data[2];
	
	status = htu21_read_user_register(&reg);
	if( status != htu21_status_ok )
		return status;
	
	// Clear bits of reg that are not reserved
	reg &= HTU21_USER_REG_RESERVED_MASK;
	// Set bits from value that are not reserved
	reg |= (value & ~HTU21_USER_REG_RESERVED_MASK);
	
	data[0] = HTU21_WRITE_USER_REG_COMMAND;
	data[1] = reg; 

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, 2, true);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;
		
	return htu21_status_ok;
}
htu21_status_t htu21_temperature_conversion_and_read_adc(uint16_t *adc)
{
	htu21_status_t status = htu21_status_ok;
	esp_err_t i2c_status;
	uint16_t _adc;
	uint8_t buffer[3];
	uint8_t crc;
	
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	
	if( i2c_master_mode == htu21_i2c_hold) {
		status = htu21_write_command_no_stop(HTU21_READ_TEMPERATURE_W_HOLD_COMMAND);
	}
	else {
		status = htu21_write_command(HTU21_READ_TEMPERATURE_WO_HOLD_COMMAND);
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	if( status != htu21_status_ok)
		return status;
		
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read(cmd, buffer, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
	
	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;

	_adc = (buffer[0] << 8) | buffer[1];
	crc = buffer[2];
	
	// compute CRC
	status = htu21_crc_check(_adc,crc);
	if( status != htu21_status_ok)
		return status;
	
	*adc = _adc;

	return status;
}
htu21_status_t htu21_humidity_conversion_and_read_adc( uint16_t *adc)
{
	htu21_status_t status = htu21_status_ok;
	esp_err_t i2c_status;
	uint16_t _adc;
	uint8_t buffer[3];
	uint8_t crc;
	
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	
	if( i2c_master_mode == htu21_i2c_hold) {
		status = htu21_write_command_no_stop(HTU21_READ_HUMIDITY_W_HOLD_COMMAND);
	}
	else {
		status = htu21_write_command(HTU21_READ_HUMIDITY_WO_HOLD_COMMAND);
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	if( status != htu21_status_ok)
		return status;
		
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();     //TODO: move to wrapper in platform
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_ADDR << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read(cmd, buffer, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_status = i2c_master_cmd_begin(I2C_NUM_0, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);

	if( i2c_status == ESP_FAIL )
		return htu21_status_no_i2c_acknowledge;
	if( i2c_status != ESP_OK)
		return htu21_status_i2c_transfer_error;

	_adc = (buffer[0] << 8) | buffer[1];
	crc = buffer[2];
	
	// compute CRC
	status = htu21_crc_check(_adc,crc);
	if( status != htu21_status_ok)
		return status;
	
	*adc = _adc;

	return status;
}
void htu21_main(void* arg)
{
	htu21_t* htu21 = (htu21_t*)arg;
	htu21_data_t data = 
	{
		.temperature = 0,
		.humidity = 0
	};
	htu21_init();
	htu21_set_resolution(htu21_resolution_t_13b_rh_10b);
	if(NULL == htu21->msg_queue)
	{
		LOG_ERROR("Queue is null");
	}
	
	while(1)
	{
		// Read temperature and humidity
		htu21_read_temperature_and_relative_humidity((&data.temperature), &(data.humidity));
		// Add to Queue
		data.temperature = data.temperature*(9.0/5.0)+32.0;
		xQueueOverwrite(htu21->msg_queue, &data);
		vTaskDelay(10); // sleep for half a second
	}
	vTaskDelete(NULL);
	return (function_cb_t)(0);
}