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

bool BrailleInput::pushBackBraille()
{
  brlBuf[0] = nabccTable[inputKey];
  return true;
}

bool BrailleInput::two_input(uint8_t key)
{
  uint16_t i=0;
  // 特殊キー処理
  if (key == 0x5) {
    brlBuf[0] = 0xB0;
    inputState=0;
    return true;
  }
  if (key == 0x6) {
    brlBuf[0] = 0xB2;
    inputState=0;
    return true;
  }

  // 入力を点字ビット列に変換
  if (key == 0x4) i = 0; // ブランク
  else {
    if (key & 0x01) i = 1;
    if (key & 0x02) i |= 0x08;
  }

  // 入力段階チェック
  if (inputState == 1) i <<= 1;
  else if (inputState == 2) i <<= 2;

  // 次の段階に進むための初期化
  inputKey |= i;
  inputState++;
  brlBuf[0] = 0;

  // 最終段階、文字の生成
  if (inputState == 3) {
    pushBackBraille();
    inputKey=0;
    inputState=0;
    return true;
  }
  return false;
}
