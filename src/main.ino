#include <Arduino.h>
#include <BleKeyboard.h>
#include "SoundOut.h"
#include "BrailleInput.hxx"

BleKeyboard bKeyboard;
BrailleInput brl;

class SwitchDriver {
public:
	void begin();
	uint16_t getKey();
private:
	uint16_t state=0;
	static const uint32_t threshold=10000;
	static uint8_t pins[9];
	uint16_t press();
	uint16_t release();
	uint16_t getstate() { return state; };
};

uint8_t SwitchDriver::pins[] = {37,38,39,0};

void SwitchDriver::begin()
{
	delay(5000);
	for (uint8_t i=0; pins[i] != 0; i++) {
		pinMode(pins[i], INPUT_PULLUP);
		Serial.print(pins[i]);
		Serial.println(" is initialized");
	}
}

uint16_t SwitchDriver::press()
{
	for (uint8_t i=0; pins[i]!=0; i++) {
		uint32_t start = micros();
		uint32_t end=0;
		uint32_t len=0;
		while(!bitRead(state, i) && !digitalRead(pins[i])) {
			end = micros();
			if (end < start) len = (0xffff-start) + end;
			else len = end-start;
			if (len > threshold) break;
		}
		if (len > threshold) {
			bitSet(state, i);
		}
	}
	return state;
}

uint16_t SwitchDriver::release()
{
	for (uint8_t i=0; pins[i]!=0; i++) {
		uint32_t start = micros();
		uint32_t end=0;
		uint32_t len=0;
		while(bitRead(state, i) && digitalRead(pins[i])) {
			end = micros();
			if (end < start) len = (0xffff-start) + end;
			else len = end-start;
			if (len > threshold) break;
		}
		if (len > threshold) bitClear(state, i);
	}
	return state;
}

uint16_t SwitchDriver::getKey()
{
	uint16_t retCode=0;
	while(!state) {
		if (retCode < state) retCode = state;
		press();
	}
	while(state) {
		if (retCode < state) retCode = state;
		press();
		release();
	}
	return retCode;
}

SwitchDriver keys;

void setup()
{
	Serial.begin(115200);
	keys.begin();
	bKeyboard.begin();
	brl.begin(two_dot_mode);
	sOut.begin();
}

void loop()
{
	uint16_t key = keys.getKey();
	if (key) {
		brl.input(key);
		uint8_t c = brl.get();
		if (c) Serial.println(c);
	}
}
