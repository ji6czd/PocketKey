#ifndef _BRaILLE_INPUT_HXX_
#define _BRaILLE_INPUT_HXX_

enum
input_mode_t {
	      eight_dot_mode,
	      six_dot_mode,
	      two_dot_mode
};

enum
language_mode_t {
		 nabcc,
		 ubc,
		 jp1
};

class BrailleInput {
public:
  bool begin(input_mode_t mode);
  bool setMode(input_mode_t mode) { return true; };
  bool setLanguage(language_mode_t mode) { return true; };
  bool input(uint16_t key);
  bool clear() { brlBuf[0]=0; return true; };
  uint8_t get() { return brlBuf[0]; };
private:
  bool six_input(uint8_t key) { return true; };
  bool eight_input(uint8_t key) { return true; };
  bool two_input(uint8_t key);
  uint16_t inputKey;
  uint8_t brlBuf[4];
  input_mode_t i_mode;
  int inputState;
};
#endif
