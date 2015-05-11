/*
  TwoWireModified.cpp - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/

//added
#define I2C_WAIT_TIMEOUT 500
//----

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__)

#include "kinetis.h"
#include <string.h> // for memcpy
#include "core_pins.h"
//#include "HardwareSerial.h"
#include "WireModified.h"
#include "Globals.h"

uint8_t TwoWireModified::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWireModified::rxBufferIndex = 0;
uint8_t TwoWireModified::rxBufferLength = 0;
//uint8_t TwoWireModified::txAddress = 0;
uint8_t TwoWireModified::txBuffer[BUFFER_LENGTH+1];
uint8_t TwoWireModified::txBufferIndex = 0;
uint8_t TwoWireModified::txBufferLength = 0;
uint8_t TwoWireModified::transmitting = 0;
void (*TwoWireModified::user_onRequest)(void) = NULL;
void (*TwoWireModified::user_onReceive)(int) = NULL;


TwoWireModified::TwoWireModified()
{
}

static uint8_t slave_mode = 0;
static uint8_t irqcount=0;


void TwoWireModified::begin(void)
{
	//serial_begin(BAUD2DIV(115200));
	//serial_print("\nWireModified Begin\n");

	slave_mode = 0;
	SIM_SCGC4 |= SIM_SCGC4_I2C0; // TODO: use bitband
	I2C0_C1 = 0;
	// On Teensy 3.0 external pullup resistors *MUST* be used
	// the PORT_PCR_PE bit is ignored when in I2C mode
	// I2C will not work at all without pullup resistors
	// It might seem like setting PORT_PCR_PE & PORT_PCR_PS
	// would enable pullup resistors.  However, there seems
	// to be a bug in chip while I2C is enabled, where setting
	// those causes the port to be driven strongly high.
	CORE_PIN18_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	CORE_PIN19_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	setClock(100000);
	I2C0_C2 = I2C_C2_HDRS;
	I2C0_C1 = I2C_C1_IICEN;
	//pinMode(3, OUTPUT);
	//pinMode(4, OUTPUT);
}

void TwoWireModified::setClock(uint32_t frequency)
{
#if F_BUS == 60000000
	if (frequency < 400000) {
		I2C0_F = 0x2C;	// 104 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x1C; // 416 kHz
	} else {
		I2C0_F = 0x12; // 938 kHz
	}
	I2C0_FLT = 4;
#elif F_BUS == 56000000
	if (frequency < 400000) {
		I2C0_F = 0x2B;	// 109 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x1C; // 389 kHz
	} else {
		I2C0_F = 0x0E; // 1 MHz
	}
	I2C0_FLT = 4;
#elif F_BUS == 48000000
	if (frequency < 400000) {
		I2C0_F = 0x27;	// 100 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x1A; // 400 kHz
	} else {
		I2C0_F = 0x0D; // 1 MHz
	}
	I2C0_FLT = 4;
#elif F_BUS == 40000000
	if (frequency < 400000) {
		I2C0_F = 0x29;	// 104 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x19; // 416 kHz
	} else {
		I2C0_F = 0x0B; // 1 MHz
	}
	I2C0_FLT = 3;
#elif F_BUS == 36000000
	if (frequency < 400000) {
		I2C0_F = 0x28;	// 113 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x19; // 375 kHz
	} else {
		I2C0_F = 0x0A; // 1 MHz
	}
	I2C0_FLT = 3;
#elif F_BUS == 24000000
	if (frequency < 400000) {
		I2C0_F = 0x1F; // 100 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x12; // 375 kHz
	} else {
		I2C0_F = 0x02; // 1 MHz
	}
	I2C0_FLT = 2;
#elif F_BUS == 16000000
	if (frequency < 400000) {
		I2C0_F = 0x20; // 100 kHz
	} else if (frequency < 1000000) {
		I2C0_F = 0x07; // 400 kHz
	} else {
		I2C0_F = 0x00; // 800 MHz
	}
	I2C0_FLT = 1;
#elif F_BUS == 8000000
	if (frequency < 400000) {
		I2C0_F = 0x14; // 100 kHz
	} else {
		I2C0_F = 0x00; // 400 kHz
	}
	I2C0_FLT = 1;
#elif F_BUS == 4000000
	if (frequency < 400000) {
		I2C0_F = 0x07; // 100 kHz
	} else {
		I2C0_F = 0x00; // 200 kHz
	}
	I2C0_FLT = 1;
#elif F_BUS == 2000000
	I2C0_F = 0x00; // 100 kHz
	I2C0_FLT = 1;
#else
#error "F_BUS must be 60, 56, 48, 40, 36, 24, 16, 8, 4 or 2 MHz"
#endif
}

void TwoWireModified::begin(uint8_t address)
{
	begin();
	I2C0_A1 = address << 1;
	slave_mode = 1;
	I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE;
	NVIC_ENABLE_IRQ(IRQ_I2C0);
}

void i2c0_isr(void)
{
	uint8_t status, c1, data;
	static uint8_t receiving=0;

	status = I2C0_S;
	//serial_print(".");
	if (status & I2C_S_ARBL) {
		// Arbitration Lost
		I2C0_S = I2C_S_ARBL;
		//serial_print("a");
		if (receiving && TwoWireModified::rxBufferLength > 0) {
			// TODO: does this detect the STOP condition in slave receive mode?


		}
		if (!(status & I2C_S_IAAS)) return;
	}
	if (status & I2C_S_IAAS) {
		//serial_print("\n");
		// Addressed As A Slave
		if (status & I2C_S_SRW) {
			//serial_print("T");
			// Begin Slave Transmit
			receiving = 0;
			TwoWireModified::txBufferLength = 0;
			if (TwoWireModified::user_onRequest != NULL) {
				TwoWireModified::user_onRequest();
			}
			if (TwoWireModified::txBufferLength == 0) {
				// is this correct, transmitting a single zero
				// when we should send nothing?  Arduino's AVR
				// implementation does this, but is it ok?
				TwoWireModified::txBufferLength = 1;
				TwoWireModified::txBuffer[0] = 0;
			}
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_TX;
			I2C0_D = TwoWireModified::txBuffer[0];
			TwoWireModified::txBufferIndex = 1;
		} else {
			// Begin Slave Receive
			//serial_print("R");
			receiving = 1;
			TwoWireModified::rxBufferLength = 0;
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE;
			data = I2C0_D;
		}
		I2C0_S = I2C_S_IICIF;
		return;
	}
	#if defined(KINETISL)
	c1 = I2C0_FLT;
	if ((c1 & I2C_FLT_STOPF) && (c1 & I2C_FLT_STOPIE)) {
		I2C0_FLT = c1 & ~I2C_FLT_STOPIE;
		if (TwoWireModified::user_onReceive != NULL) {
			TwoWireModified::rxBufferIndex = 0;
			TwoWireModified::user_onReceive(TwoWireModified::rxBufferLength);
		}
	}
	#endif
	c1 = I2C0_C1;
	if (c1 & I2C_C1_TX) {
		// Continue Slave Transmit
		//serial_print("t");
		if ((status & I2C_S_RXAK) == 0) {
			//serial_print(".");
			// Master ACK'd previous byte
			if (TwoWireModified::txBufferIndex < TwoWireModified::txBufferLength) {
				I2C0_D = TwoWireModified::txBuffer[TwoWireModified::txBufferIndex++];
			} else {
				I2C0_D = 0;
			}
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE | I2C_C1_TX;
		} else {
			//serial_print("*");
			// Master did not ACK previous byte
			I2C0_C1 = I2C_C1_IICEN | I2C_C1_IICIE;
			data = I2C0_D;
		}
	} else {
		// Continue Slave Receive
		irqcount = 0;
		#if defined(KINETISK)
		attachInterrupt(18, TwoWireModified::sda_rising_isr, RISING);
		#elif defined(KINETISL)
		I2C0_FLT |= I2C_FLT_STOPIE;
		#endif
		data = I2C0_D;
		if (TwoWireModified::rxBufferLength < BUFFER_LENGTH && receiving) {
			TwoWireModified::rxBuffer[TwoWireModified::rxBufferLength++] = data;
		}
	}
	I2C0_S = I2C_S_IICIF;
}

// Detects the stop condition that terminates a slave receive transfer.
// Sadly, the I2C in Kinetis K series lacks the stop detect interrupt
// This pin change interrupt hack is needed to detect the stop condition
void TwoWireModified::sda_rising_isr(void)
{
	if (!(I2C0_S & I2C_S_BUSY)) {
		detachInterrupt(18);
		if (user_onReceive != NULL) {
			rxBufferIndex = 0;
			user_onReceive(rxBufferLength);
		}
		//delayMicroseconds(100);
	} else {
		if (++irqcount >= 2 || !slave_mode) {
			detachInterrupt(18);
		}
	}
}


// Chapter 44: Inter-Integrated Circuit (I2C) - Page 1012
//  I2C0_A1      // I2C Address Register 1
//  I2C0_F       // I2C Frequency Divider register
//  I2C0_C1      // I2C Control Register 1
//  I2C0_S       // I2C Status register
//  I2C0_D       // I2C Data I/O register
//  I2C0_C2      // I2C Control Register 2
//  I2C0_FLT     // I2C Programmable Input Glitch Filter register


static uint8_t i2c_wait(void)
{
	//spin
	digitalWriteFast(1,HIGH);
	unsigned long waitStart = micros();
	while(!(I2C0_S & I2C_S_IICIF)){  // wait
		if(micros() - waitStart > I2C_WAIT_TIMEOUT){
			return 1;
		}
	}
	digitalWriteFast(1,LOW);
	I2C0_S = I2C_S_IICIF;
	return 0;
}

void TwoWireModified::beginTransmission(uint8_t address)
{
	txBuffer[0] = (address << 1);
	transmitting = 1;
	txBufferLength = 1;
}

size_t TwoWireModified::write(uint8_t data)
{
	if (transmitting || slave_mode) {
		if (txBufferLength >= BUFFER_LENGTH+1) {
			setWriteError();
			return 0;
		}
		txBuffer[txBufferLength++] = data;
		return 1;
	}
	return 0;
}

size_t TwoWireModified::write(const uint8_t *data, size_t quantity)
{
	if (transmitting || slave_mode) {
		size_t avail = BUFFER_LENGTH+1 - txBufferLength;
		if (quantity > avail) {
			quantity = avail;
			setWriteError();
		}
		memcpy(txBuffer + txBufferLength, data, quantity);
		txBufferLength += quantity;
		return quantity;
	}
	return 0;
}

void TwoWireModified::flush(void)
{
}


uint8_t TwoWireModified::endTransmission(uint8_t sendStop)
{
	uint8_t i, status, ret=0;

	// clear the status flags
	I2C0_S = I2C_S_IICIF | I2C_S_ARBL;
	// now take control of the bus...
	if (I2C0_C1 & I2C_C1_MST) {
		// we are already the bus master, so send a repeated start
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
	} else {
		// we are not currently the bus master, so wait for bus ready
		unsigned long waitStart = micros();
		while(I2C0_S & I2C_S_BUSY) {	//spin
			if(micros() - waitStart > I2C_WAIT_TIMEOUT){
				return 5;
			}
		}
		// become the bus master in transmit mode (send start)
		slave_mode = 0;
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	}
	// transmit the address and data
	for (i=0; i < txBufferLength; i++) {
		I2C0_D = txBuffer[i];
		if(i2c_wait()){
			return 6;
		}
		status = I2C0_S;
		if (status & I2C_S_RXAK) {
			// the slave device did not acknowledge
			if (i == 0) {
				ret = 2; // 2:received NACK on transmit of address
			} else {
				ret = 3; // 3:received NACK on transmit of data 
			}
			break;
		}
		if ((status & I2C_S_ARBL)) {
			// we lost bus arbitration to another master
			// TODO: what is the proper thing to do here??
			ret = 4; // 4:other error 
			break;
		}
	}
	if (sendStop) {
		// send the stop condition
		I2C0_C1 = I2C_C1_IICEN;
		// TODO: do we wait for this somehow?
	}
	transmitting = 0;
	return ret;
}


uint8_t TwoWireModified::requestFrom(uint8_t address, uint8_t length, uint8_t sendStop)
{
	uint8_t tmp __attribute__((unused));
	uint8_t status, count=0;

	rxBufferIndex = 0;
	rxBufferLength = 0;
	// clear the status flags
	I2C0_S = I2C_S_IICIF | I2C_S_ARBL;
	// now take control of the bus...
	if (I2C0_C1 & I2C_C1_MST) {
		// we are already the bus master, so send a repeated start
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
	} else {
		// we are not currently the bus master, so wait for bus ready
		unsigned long waitStart = micros();
		while(I2C0_S & I2C_S_BUSY) {	//spin
			if(micros() - waitStart > I2C_WAIT_TIMEOUT){
				return 0;
			}
		}
		// become the bus master in transmit mode (send start)
		slave_mode = 0;
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	}
	// send the address
	I2C0_D = (address << 1) | 1;
	if(i2c_wait()){
		return 0;
	}
	status = I2C0_S;
	if ((status & I2C_S_RXAK) || (status & I2C_S_ARBL)) {
		// the slave device did not acknowledge
		// or we lost bus arbitration to another master
		I2C0_C1 = I2C_C1_IICEN;
		return 0;
	}
	if (length == 0) {
		// TODO: does anybody really do zero length reads?
		// if so, does this code really work?
		I2C0_C1 = I2C_C1_IICEN | (sendStop ? 0 : I2C_C1_MST);
		return 0;
	} else if (length == 1) {
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
	} else {
		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST;
	}
	tmp = I2C0_D; // initiate the first receive
	while (length > 1) {
		if(i2c_wait()){
			return 0;
		}
		length--;
		if (length == 1) I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
		rxBuffer[count++] = I2C0_D;
	}
	if(i2c_wait()){
		return 0;
	}
	I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	rxBuffer[count++] = I2C0_D;
	if (sendStop) I2C0_C1 = I2C_C1_IICEN;
	rxBufferLength = count;
	return count;
}

int TwoWireModified::available(void)
{
	return rxBufferLength - rxBufferIndex;
}

int TwoWireModified::read(void)
{
	if (rxBufferIndex >= rxBufferLength) return -1;
	return rxBuffer[rxBufferIndex++];
}

int TwoWireModified::peek(void)
{
	if (rxBufferIndex >= rxBufferLength) return -1;
	return rxBuffer[rxBufferIndex];
}





// alternate function prototypes

uint8_t TwoWireModified::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWireModified::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWireModified::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWireModified::beginTransmission(int address)
{
	beginTransmission((uint8_t)address);
}

uint8_t TwoWireModified::endTransmission(void)
{
	return endTransmission(true);
}

void TwoWireModified::begin(int address)
{
	begin((uint8_t)address);
}

void TwoWireModified::onReceive( void (*function)(int) )
{
	user_onReceive = function;
}

void TwoWireModified::onRequest( void (*function)(void) )
{
	user_onRequest = function;
}

//TwoWireModified WireModified = TwoWireModified();
TwoWireModified WireModified;


#endif // __MK20DX128__ || __MK20DX256__



#if defined(__AVR__)

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #include "twi.h"
}

#include "WireModified.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWireModified::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWireModified::rxBufferIndex = 0;
uint8_t TwoWireModified::rxBufferLength = 0;

uint8_t TwoWireModified::txAddress = 0;
uint8_t TwoWireModified::txBuffer[BUFFER_LENGTH];
uint8_t TwoWireModified::txBufferIndex = 0;
uint8_t TwoWireModified::txBufferLength = 0;

uint8_t TwoWireModified::transmitting = 0;
void (*TwoWireModified::user_onRequest)(void);
void (*TwoWireModified::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////

TwoWireModified::TwoWireModified()
{
}

// Public Methods //////////////////////////////////////////////////////////////

void TwoWireModified::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  twi_init();
}

void TwoWireModified::begin(uint8_t address)
{
  twi_setAddress(address);
  twi_attachSlaveTxEvent(onRequestService);
  twi_attachSlaveRxEvent(onReceiveService);
  begin();
}

void TwoWireModified::begin(int address)
{
  begin((uint8_t)address);
}

void TwoWireModified::setClock(uint32_t frequency)
{
  TWBR = ((F_CPU / frequency) - 16) / 2;
}

uint8_t TwoWireModified::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform blocking read into buffer
  uint8_t read = twi_readFrom(address, rxBuffer, quantity, sendStop);
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t TwoWireModified::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWireModified::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWireModified::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWireModified::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void TwoWireModified::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

//
//	Originally, 'endTransmission' was an f(void) function.
//	It has been modified to take one parameter indicating
//	whether or not a STOP should be performed on the bus.
//	Calling endTransmission(false) allows a sketch to 
//	perform a repeated start. 
//
//	WARNING: Nothing in the library keeps track of whether
//	the bus tenure has been properly ended with a STOP. It
//	is very possible to leave the bus in a hung state if
//	no call to endTransmission(true) is made. Some I2C
//	devices will behave oddly if they do not see a STOP.
//
uint8_t TwoWireModified::endTransmission(uint8_t sendStop)
{
  // transmit buffer (blocking)
  int8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1, sendStop);
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return ret;
}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t TwoWireModified::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWireModified::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      setWriteError();
      return 0;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(&data, 1);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWireModified::write(const uint8_t *data, size_t quantity)
{
  if(transmitting){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(data, quantity);
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWireModified::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWireModified::read(void)
{
  int value = -1;
  
  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWireModified::peek(void)
{
  int value = -1;
  
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
  }

  return value;
}

void TwoWireModified::flush(void)
{
  // XXX: to be implemented.
}

// behind the scenes function that is called when data is received
void TwoWireModified::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];    
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void TwoWireModified::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}

// sets function called on slave write
void TwoWireModified::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void TwoWireModified::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

TwoWireModified WireModified = TwoWireModified();

#endif // __AVR__
