#include "AttitudeMeasurement.h"

bool on = false;

void setup()
{
	pinMode(13,OUTPUT);
}

void loop()
{
	AttitudeMeasurement attitudeMeasurement(AttitudeMeasurement::PITCH, AttitudeMeasurement::ROLL, AttitudeMeasurement::YAW);
	for (;;){
		attitudeMeasurement.update();

		if (on) digitalWriteFast(13,HIGH);
		else digitalWriteFast(13, LOW);

		on = !on;
	}
}
