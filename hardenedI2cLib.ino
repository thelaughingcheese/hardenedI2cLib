#include "AttitudeMeasurement.h"
#include "WritePins.h"

bool on = false;

void setup()
{
	pinMode(13,OUTPUT);
	pinMode(12, OUTPUT);
}

void loop()
{
	AttitudeMeasurement attitudeMeasurement(AttitudeMeasurement::PITCH, AttitudeMeasurement::ROLL, AttitudeMeasurement::YAW);
	for (;;){
		WritePins(1);

		attitudeMeasurement.update();

		WritePins(255);

		if (on) digitalWriteFast(12,HIGH);
		else digitalWriteFast(12, LOW);

		on = !on;
	}
}
