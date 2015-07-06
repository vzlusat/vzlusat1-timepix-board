/*
 * ADT7420.h
 *
 * Created: 22/02/2015 17:29:48
 *  Author: Ondra
 */ 


#ifndef ADT7420_H_
#define ADT7420_H_

enum ADT_commands_enum {

	ADT_REG_TEMPERATURE			= 0x00,
	ADT_REG_TEMPERATURE_MSB		= 0x00,
	ADT_REG_TEMPERATURE_LSB		= 0x01,
	ADT_REG_STATUS				= 0x02,
	ADT_REG_CONFIGURATION		= 0x03,
	ADT_REG_T_HIGH				= 0x04,
	ADT_REG_T_HIGH_MSB			= 0x04,
	ADT_REG_T_HIGH_LSB			= 0x05,
	ADT_REG_T_LOW				= 0x06,
	ADT_REG_T_LOW_MSB			= 0x06,
	ADT_REG_T_LOW_LSB			= 0x07,
	ADT_REG_T_CRIT				= 0x08,
	ADT_REG_T_CRIT_MSB			= 0x08,
	ADT_REG_T_CRIT_LSB			= 0x09,
	ADT_REG_T_HYST				= 0x0A,
	ADT_REG_ID					= 0x0B,
	ADT_REG_SW_RESET			= 0x2F,	
} ADT_commands;


void ADT_init(void);
int8_t ADT_get_temperature(void);
static inline float ADT_temperature_16b(int temperature);

#endif /* ADT7420_H_ */