/* 
 * File:   AT24Cxx.h
 * Author: Manjunath CV
 *
 * Created on February 16, 2016, 12:19 AM
 */

#ifndef AT24CXX_H
#define AT24CXX_H

#include <Arduino.h>

class AT24Cxx {
public:
	AT24Cxx(uint8_t i2c_address);
	AT24Cxx(uint8_t i2c_address, uint32_t eeprom_size);
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);
	void update(uint16_t address, uint8_t value);
	uint32_t length(void);

	int operator[](uint16_t address);  // Used to access
	
protected:
	uint32_t eeprom_size;
	uint8_t i2c_address;
};

#endif /* AT24CXX_H */

