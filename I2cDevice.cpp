#include "I2cDevice.h"

I2cDevice::I2cDevice(uint8_t address){
	//start wire
	Wire.begin();

	deviceAddress = address;
}

void I2cDevice::writeRegister(uint8_t reg,uint8_t val){
	Wire.beginTransmission(deviceAddress);
	Wire.write(reg);
	Wire.write(val);
	Wire.endTransmission();
}

uint8_t I2cDevice::readRegister(uint8_t reg){
	Wire.beginTransmission(deviceAddress);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(deviceAddress,1);
	while(Wire.available()<1);
	uint8_t rtn = Wire.read();
	Wire.endTransmission();
	return rtn;
}

bool I2cDevice::isReady(){
	return ready;
}