#ifndef ATTITUDEMEASUREMENT
#define ATTITUDEMEASUREMENT

#include <Arduino.h>
#include <math.h>
#include "Accelerometer.h"
#include "Gyroscope.h"
#include "Magnetometer.h"
#include "LowPassSensorFilter.h"

class AttitudeMeasurement{
public:
	typedef enum{
		PITCH = 1,
		ROLL = 2,
		YAW = 3
	} Axis;
private:
	Accelerometer accelerometer;
	Gyroscope gyroscope;
	Magnetometer magnetometer;

        LowPassSensorFilter<short> accelXFilter;
        LowPassSensorFilter<short> accelYFilter;
        LowPassSensorFilter<short> accelZFilter;
        
        short accelX;
        short accelY;
        short accelZ;

        LowPassSensorFilter<short> gryoYFilter;
        
        short gyroY;

	float pitch,roll,yaw;
	Axis xMapping;
	Axis yMapping;
	Axis zMapping;
	uint32_t lastUpdate;
public:
	AttitudeMeasurement(Axis X,Axis Y,Axis Z);

	//rate
	float getAxisAngleRate(Axis axis);

	//absolute
	float getAccelerometerAxisAngle(Axis axis);
	float getAxisAngleAbsolute(Axis axis);

	void update();
};

#endif
