#include "AttitudeMeasurement.h"
#include "WritePins.h"
#include "WireModified.h"

bool on = false;
unsigned long lastLight = 0;

void setup()
{
	pinMode(13,OUTPUT);
	pinMode(12, OUTPUT);

	pinMode(0,OUTPUT);
	pinMode(1,OUTPUT);
	pinMode(2,OUTPUT);
	pinMode(3,OUTPUT);
	pinMode(4,OUTPUT);
	pinMode(5,OUTPUT);
	pinMode(6,OUTPUT);
	pinMode(7,OUTPUT);
	delay(100);

	pinMode(20,INPUT);
	digitalWriteFast(20,HIGH);
}

void loop()
{
	digitalWriteFast(12,HIGH);
	AttitudeMeasurement attitudeMeasurement(AttitudeMeasurement::PITCH, AttitudeMeasurement::ROLL, AttitudeMeasurement::YAW);
	for (;;){
		WritePins(1);
		attitudeMeasurement.update();

		WritePins(255);

		if (on) digitalWriteFast(12,HIGH);
		else digitalWriteFast(12, LOW);
		if(millis() - lastLight > 200){
			on = !on;
			lastLight = millis();
		}
	}
}
