#define RESET_BUS_AND_BREAK Wire.begin(); \
digitalWriteFast(13,HIGH); \
	continue;

#include "Magnetometer.h"

Magnetometer::Magnetometer(): I2cDevice(LSM303_ADDRESS_MAG){
	//enable magnetometer at 220hz
	writeRegister(LSM303_REGISTER_MAG_CRA_REG_M,LSM303_MAGRATE_220 << 2);

	//set to continuous conversion
	writeRegister(LSM303_REGISTER_MAG_MR_REG_M,0x00);

	//assume its all good
	//uint8_t reg1_a = readRegister(LSM303_REGISTER_ACCEL_CTRL_REG1_A);
	//DEBUGSPRINTLN((reg1_a,HEX));

	//set gain
	
	writeRegister(LSM303_REGISTER_MAG_CRB_REG_M,LSM303_MAGGAIN_1_3 << 4);

	xOffset = 0;
	yOffset = 0;
	zOffset = 0;

	ready = true;
}

//handle errors here!!
void Magnetometer::update(){
	for(;;){
		Wire.beginTransmission(deviceAddress);
		Wire.write(LSM303_REGISTER_MAG_OUT_X_H_M | 0x80);
		if(Wire.endTransmission()){
			RESET_BUS_AND_BREAK
		}

		if(!Wire.requestFrom(deviceAddress,6)){
			RESET_BUS_AND_BREAK
		}
		while(Wire.available() < 6);
		x = (Wire.read() << 8 | Wire.read()) - xOffset;
		y = (Wire.read() << 8 | Wire.read()) - yOffset;
		z = (Wire.read() << 8 | Wire.read()) - zOffset;

		if(Wire.endTransmission()){
			RESET_BUS_AND_BREAK
		}
		break;
	}
}

void Magnetometer::calibrateOffset(){
	//DEBUGSPRINTLN("magnetometer calibration not implemented");
}
