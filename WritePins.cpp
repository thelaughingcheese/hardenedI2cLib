#include "Arduino.h"

void WritePins(unsigned char num){
	digitalWriteFast(0,(num & 0x01)); num >>= 1;
	digitalWriteFast(1,(num & 0x01)); num >>= 1;
	digitalWriteFast(2,(num & 0x01)); num >>= 1;
	digitalWriteFast(3,(num & 0x01)); num >>= 1;
	digitalWriteFast(4,(num & 0x01)); num >>= 1;
	digitalWriteFast(5,(num & 0x01)); num >>= 1;
	//digitalWriteFast(6,(num & 0x01)); num >>= 1;
	digitalWriteFast(7,(num & 0x01)); num >>= 1;
}