/*
Braille Input routine
*/

#include "BrailleInput.hxx"

class BrailleTranslation {
public:
  virtual bool translation(uint8_t brlBuf[], std::string &outStr) { outStr = "test"; return true; };
  virtual bool setMode(uint8_t mode) { return true; };
};

class BrailleTranslationJP : public BrailleTranslation {
public:
  bool translation(uint8_t brlBuf[], std::string &outStr) { outStr = "jp"; return true; };
};

class BrailleTranslationEN : public BrailleTranslation {
public:
  BrailleTranslationEN() { mode = normal; };
  bool translation(uint8_t brlBuf[], std::string &outStr);
private:
  enum
  engbrlMode {
	      normal,
	      caps,
	      dobuecaps,
	      numeric
  };
  engbrlMode mode;
};

bool BrailleTranslationEN::translation(uint8_t brlBuf[], std::string &outStr)
{
  // 軽くモード設定
  if (brlBuf[0] == ',') {
    mode = caps;
    return false;
  }
  else if (brlBuf[0] == '#') {
    mode=numeric;
    return false;
  }
  else if (brlBuf[0] == ' ') mode=normal;
  // 軽く変換
  if (mode == normal) {
    outStr.push_back(brlBuf[0]);
  }
  else if (mode == numeric && brlBuf[0] >= 'a') {
    outStr.push_back(brlBuf[0]-0x30);
  }
  else if (mode == caps) {
    outStr.push_back(brlBuf[0]-0x20);
    mode = normal;
  }
  return true;
}

BrailleTranslation *pBt;
BrailleTranslationJP btJP;
BrailleTranslationEN btEN;

static char nabccTable[] =
  " a1b'k;l`cif/msp\"e3h9o6r~djg>ntq,*5<-u8v.%{$+x!&;:4|0z7(_?w}#y)=";

bool BrailleInput::begin(input_mode_t mode)
{
  i_mode = mode;
  inputState=0;
  inputKey=0;
  pBt = &btEN;
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
  int i;
  for (i = 0; brlBuf[i] && i < 6; i++) {
    ; // 文字が入っている部分をスキップ
  }
  brlBuf[i] = nabccTable[inputKey];
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
  outStr.clear();;

  // 最終段階、文字の生成
  if (inputState == 3) {
    pushBackBraille();
    pBt->translation(brlBuf, outStr);
    inputKey=0;
    inputState=0;
    return true;
  }
  return false;
}
