#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include "BrailleInput.hxx"
#include "delay.h"

BrailleInput brl;

class SwitchDriver {
public:
	void begin();
	uint16_t getKey();
private:
	uint16_t state=0;
	static const uint32_t threshold=10000;
	static uint8_t pins[8];
	uint16_t press();
	uint16_t release();
	uint16_t getstate() { return state; };
};

uint8_t SwitchDriver::pins[] = {9, 10, 11, 12, 13};

void SwitchDriver::begin()
{
	for (int i = 0; i < 10; i++) {
		Serial.printf("Initializing... %d\n", i);
		delay(1000);
	}
	for (uint8_t i=0; pins[i] != 0; i++) {
		pinMode(pins[i], INPUT_PULLUP);
		Serial.printf("Pin %d initialized.\n", pins[i]);
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
	brl.begin(two_dot_mode);
}

void loop()
{
	uint16_t key = keys.getKey();
	if (key) {
		brl.input(key);
		const char *s = brl.get();
		if (s[0]) {
			Serial.print(s);
		}
	}
}
