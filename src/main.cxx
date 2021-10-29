#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial
#include <bluefruit.h>

#include "BrailleInput.hxx"

BrailleInput brl;
BLEDis bledis;
BLEHidAdafruit blehid;
bool hasKeyPressed = false;

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
	for (int i = 0; i < 5; i++) {
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

/**
 * Callback invoked when received Set LED from central.
 * Must be set previously with setKeyboardLedCallback()
 *
 * The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
 *    Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
 */
void set_keyboard_led(uint16_t conn_handle, uint8_t led_bitmap)
{
  (void) conn_handle;
  
  // light up Red Led if any bits is set
  if ( led_bitmap )
  {
    ledOn( LED_RED );
  }
  else
  {
    ledOff( LED_RED );
  }
}

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 HID Keyboard Example");
  Serial.println("--------------------------------\n");

  Serial.println();
  Serial.println("Go to your phone's Bluetooth settings to pair your device");
  Serial.println("then open an application that accepts keyboard input");

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Braille Pocket Keyboard");
  bledis.begin();

  /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval 
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
   * connection interval to 11.25  ms and 15 ms respectively for best performance.
   */
  blehid.begin();

  // Set callback for set LED from central
  blehid.setKeyboardLedCallback(set_keyboard_led);

  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms 
   */
  /* Bluefruit.Periph.setConnInterval(9, 12); */

  // Set up and start advertising
  startAdv();
  keys.begin();
	brl.begin(two_dot_mode);
}

void loop()
{
  // Only send KeyRelease if previously pressed to avoid sending
  // multiple keyRelease reports (that consume memory and bandwidth)
  if ( hasKeyPressed )
  {
    hasKeyPressed = false;
    blehid.keyRelease();
    
    // Delay a bit after a report
    delay(5);
  }

  uint16_t key = keys.getKey();
	if (key) {
		brl.input(key);
		const char *s = brl.get();
		if (s[0]) {
			blehid.keyPress(s[0]);
			hasKeyPressed = true;
			Serial.write(s[0]);
			// Delay a bit after a report
			delay(5);
		}
	}
}
