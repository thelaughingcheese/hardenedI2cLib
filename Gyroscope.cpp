#include "Gyroscope.h"
#include "WritePins.h"

#define RESET_BUS_AND_BREAK Wire.begin(); \
digitalWriteFast(13,HIGH); \
continue;

float Gyroscope::convertAngularVelocity(int16_t angularVelocity){
	int wordSignedMaxValue = 0x7fff;

	float maxVelocity;
        //datasheet values readjusted
	if(maxAngularVelocity == DPS245){
		maxVelocity = 310;  //245
	}
	else if(maxAngularVelocity == DPS500){
		maxVelocity = 500;
	}
	else{
		maxVelocity = 2000;
	}

	return (maxVelocity*angularVelocity) / wordSignedMaxValue;
}

//configure
Gyroscope::Gyroscope(): I2cDevice(L2GD20H_ADDRESS){
	//check if device is correct
	uint8_t who = readRegister(GYRO_REGISTER_WHO_AM_I);
	if(who != 0xd7){
		//DEBUGSPRINTLN("expecting gyroscope! invalid device");
		//DEBUGSPRINTLN((who,HEX));
		ready = false;
		return;
	}

	//clear and set config reg 1
	writeRegister(GYRO_REGISTER_CTRL_REG1,0x00);
	writeRegister(GYRO_REGISTER_CTRL_REG1,0xff);

	//set config reg 4, default 245 dps
	maxAngularVelocity = DPS245;
	writeRegister(GYRO_REGISTER_CTRL_REG4,maxAngularVelocity);

	x=0;y=0;z=0;
	xOffset=0;yOffset=0;zOffset=0;

	ready = true;
}

void Gyroscope::setMaxAngularVelocity(Gyroscope::MaxAngularVelocity max){
	maxAngularVelocity = max;
	writeRegister(GYRO_REGISTER_CTRL_REG4,max);
}

//!!we need to handle errors here!
void Gyroscope::update(){
	for(;;){
		WritePins(2);
		Wire.beginTransmission(deviceAddress);
		Wire.write(GYRO_REGISTER_OUT_X_L | 0x80);
		if(Wire.endTransmission()){
			RESET_BUS_AND_BREAK
		}
		WritePins(3);

		if(!Wire.requestFrom(deviceAddress,6)){
			RESET_BUS_AND_BREAK
		}
		WritePins(4);

		while(Wire.available() < 6);		//sholuld never seize up with teensy
		WritePins(5);									//since requestFrom blocks until all data received

		x = (Wire.read() | Wire.read() << 8) - xOffset;
		y = (Wire.read() | Wire.read() << 8) - yOffset;
		z = (Wire.read() | Wire.read() << 8) - zOffset;

		WritePins(6);
		if(Wire.endTransmission()){
			RESET_BUS_AND_BREAK
		}
		WritePins(7);
		break;
	}
}

void Gyroscope::calibrateOffset(){
	long xSum = 0;
	long ySum = 0;
	long zSum = 0;

	//first few samples are junk
	for(int i=0; i<CALIBRATION_JUNK; i++){
		update();
	}

	for(int i=0; i<CALIBRATION_SAMPLES;i++){
		update();
		xSum+=x;
		ySum+=y;
		zSum+=z;
	}

	xOffset = xSum/CALIBRATION_SAMPLES;
	yOffset = ySum/CALIBRATION_SAMPLES;
	zOffset = zSum/CALIBRATION_SAMPLES;
}
