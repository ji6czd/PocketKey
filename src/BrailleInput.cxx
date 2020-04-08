/*
Braille Input routine
*/

#include <Arduino.h>
#include "BrailleInput.hxx"

static char nabccTable[] =
  " a1b'k;l`cif/msp\"e3h9o6r~djg>ntq,*5<-u8v.%{$+x!&;:4|0z7(_?w}#y)=";

bool BrailleInput::begin(input_mode_t mode)
{
  i_mode = mode;
  inputState=0;
  inputKey=0;
  return true;
}

bool BrailleInput::input(uint16_t key)
{
  if (i_mode == eight_dot_mode) return eight_input(key);
  if (i_mode == two_dot_mode) return two_input(key);
  return false;
  if (i_mode == six_dot_mode) return six_input(key);
}
bool BrailleInput::two_input(uint8_t key)
{
  uint16_t i=0;
  if (key & 0x01) i = 1;
  if (key & 0x02) i |= 0x08;
  if (inputState == 1) i <<= 1;
  if (inputState == 2) i <<= 2;
  inputKey |= i;
  inputState++;
  brlBuf[0] = 0;
  if (inputState == 3) {
    Serial.println(inputKey);
    brlBuf[0] = nabccTable[inputKey];
    inputKey=0;
    inputState=0;
    return true;
  }
  return false;
}
