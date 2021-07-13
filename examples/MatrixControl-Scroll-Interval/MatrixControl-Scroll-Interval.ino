/*

  Scroll alphanumeric characters on 8x8 Dot Matrix LED via MAX7219.
  This project is modeled this library.
  https://github.com/wayoda/LedControl

  This project uses this font file.
  https://github.com/dhepper/font8x8
  
*/

#include "MatrixControl.h"
#include "font8x8_basic.h"

#define numDevices 4

#define interval 50;

#if defined(__AVR__)
/*
 Now we need a MatrixControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have 4 MAX72XX.
 */
MatrixControl lc=MatrixControl(12,11,10,numDevices);

#elif defined(ESP8266)
/*
 Now we need a MatrixControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 GPIO13(D7) is connected to the DataIn 
 GPIO14(D5) is connected to the CLK 
 GPIO15(D8) is connected to LOAD 
 We have 4 MAX72XX.
 */
MatrixControl lc=MatrixControl(13,14,15,numDevices);

#endif

// 
// ビットマップ関連の関数は
// http://nuneno.cocolog-nifty.com/blog/2016/07/48x8led-0f0f.html 
// のmax7219_test.zipから借用しました
//


// 任意サイズバッファのスクロール
//  bmp: スクロール対象バッファ
//  w:   バッファの幅(ドット)
//  h:   バッファの高さ(ドット)
//  mode: B0001 左 ,B0010 右, B0100 上, B1000 下 … OR で同時指定可能
void scrollBitmap(uint8_t *bmp, uint16_t w, uint16_t h, uint8_t mode) {
  uint16_t bl = (w+7)>>3;           // 横バイト数
  uint16_t addr;                    // データアドレス
  uint8_t prv_bit;
  uint8_t d;

  // 横
  if ((mode & B11) == B01) {  // 左スクロール
    addr=0;
    for (uint8_t i=0; i < h;i++) {
      prv_bit = 0;
      for (int8_t j=0; j < bl; j++) {
        d = bmp[addr+bl-1-j];
        bmp[addr+bl-1-j]<<=1;
        if (j>0)
          bmp[addr+bl-1-j] |= prv_bit;
        prv_bit=d>>7;
      }
      addr+=bl;
    }
  } else if ((mode & B11) == B10) { // 右スクロール
    addr=0;
    for (uint8_t i=0; i < h;i++) {
      prv_bit = 0;
      for (int8_t j=0; j < bl; j++) {
        d = bmp[addr+j];
        bmp[addr+j]>>=1;
        if (j>0)
          bmp[addr+j] |= prv_bit;
        prv_bit=d<<7;
        
       // Serial.print("addr+j=");
       // Serial.println(addr+j,DEC);
      }
      addr+=bl;
    } 
  }

  // 縦
  if ((mode & B1100) == B0100) {  // 上スクロール
    addr=0;   
    for (uint8_t i=0; i<h-1;i++) {
      for (int8_t j=0; j < bl; j++) {
        bmp[addr+j] = bmp[addr+j+bl];
      }
      addr+=bl;
    }
    for (int8_t j=0; j < bl; j++) {
      bmp[addr+j] = 0;
    }
  } else if ((mode & B1100) == B1000) { // 下スクロール
    addr=bl*(h-1);
    for (uint8_t i=0; i<h-1;i++) {
      for (int8_t j=0; j < bl; j++) {
        bmp[addr+j] = bmp[addr-bl+j];
      }
      addr-=bl;
    }
    for (int8_t j=0; j < bl; j++) {
      bmp[j] = 0;
    } 
  }
}

// 任意サイズのビットマップにドットをセット
void setDotAt(uint8_t* bmp, uint16_t w, uint16_t h, int16_t x, int16_t y, uint8_t d) {
  if (x < 0 || y < 0 || x >= w || y >= h)
    return;

  uint16_t bl = (w+7)>>3;           // 横必要バイト数
  uint16_t addr = bl*y + (x>>3);    // 書込みアドレス
  if (d) 
    bmp[addr] |= 0x80>>(x%8);
  else
    bmp[addr] &= ~(0x80>>(x%8));
}

// 任意サイズビットマップの指定座標のドットを取得
//  bmp:  スクロール対象バッファ
//  w:    バッファの幅(ドット)
//  h:    バッファの高さ(ドット)
//  x,y:  座標
uint8_t getdotBitmap(uint8_t *bmp, uint16_t w, uint16_t h, int16_t x, int16_t y) {
  if (x>=w || y>=h || x <0 ||  y < 0) 
    return 0;

  uint16_t bl = (w+7)>>3;           // 横バイト数    
  uint8_t d;
  
  d = bmp[y*bl + (x/8)];
  if (d & (0x80>>(x%8)))
    return 1;
  else 
    return 0;
}

// 任意サイズビットマップの回転
//  bmp:  スクロール対象バッファ
//  w:    バッファの幅(ドット)
//  h:    バッファの高さ(ドット)
//  mode: B00 なし, B01 反時計90° B10 反時計180° B11 反時計270°  
void rotateBitmap(uint8_t *bmp, uint16_t w, uint16_t h, uint8_t mode) {
  if (mode == B00 || w != h || mode > B11)
   return;  

  uint16_t bl = (w+7)>>3;           // 横バイト数  
  uint8_t tmpbmp[h*bl];
  uint8_t d;
  memset(tmpbmp,0,h*bl);
  if (mode == B01) {  // 反時計90°
    for (int16_t x = 0; x < w; x++) {
      for (int16_t y = 0; y < h; y++) {
       d = getdotBitmap(bmp, w, h, x, y);
       setDotAt(tmpbmp, w, h, w-y-1, x, d);
      }
    }
  } else if (mode == B10) { // 反時計180°
    for (int16_t x = 0; x < w; x++) {
      for (int16_t y = 0; y < h; y++) {
       d = getdotBitmap(bmp, w, h, x, y);
       setDotAt(tmpbmp, w, h, w-x-1, h-y-1, d);
      }
    }
  } else {  // 反時計270°
    for (int16_t x = 0; x < w; x++) {
      for (int16_t y = 0; y < h; y++) {
       d = getdotBitmap(bmp, w, h, x, y);
       setDotAt(tmpbmp, w, h, y, h-x-1,d);
      }
    }
  }
  memcpy(bmp,tmpbmp,h*bl);
}

// ビットマップを指定の列に表示
//  bitmap:  表示対象バッファ
//  mapside: バッファの大きさ(バイト)
//  row:     表示する列番号(-7から31)
//  maxDev:  接続されている最大デバイス数
//  invert:  反転するかどうか
int showBitmap(uint8_t *bitmap, int mapsize, int row, int maxDev, bool invert) {
  int addr = ((row+8) / 8) - 1;
  int start = 0;
  int width = 8 - ((row+8) % 8);
  int scroll = 8 - width;
  if (row < 0) {
    addr = row / 8;
    width = 8;
    start = -row;
    scroll = -row;
  }

#if 0
  Serial.print("row=");
  Serial.print(row);
  Serial.print(" invert=");
  Serial.print(invert);
  Serial.print(" addr=");
  Serial.print(addr);
  Serial.print(" start=");
  Serial.print(start);
  Serial.print(" width=");
  Serial.print(width);
  Serial.print(" scroll=");
  Serial.print(scroll);
  Serial.println();
#endif

  if (addr >= maxDev) return addr;
  if (addr < 0) return addr;

  uint8_t _bitmap[8];
  for(int i=0;i<mapsize;i++) {
    _bitmap[i] = bitmap[i];
    if (invert) _bitmap[i] = ~bitmap[i];
  }
  
  if (row >=0) {
    //上方向にスクロール
    for (int i=0;i<scroll;i++) {
      scrollBitmap(_bitmap, 8, 8, B0100);
    }
  } else {
    //下方向にスクロール
    for (int i=0;i<scroll;i++) {
      scrollBitmap(_bitmap, 8, 8, B1000);
    }
  }

  //１列単位に表示
  // col = 0 : 右端
  // col = 7 : 左端
  //byte rowData;
  //for(int i=0;i<8;i++) {
  //for(int i=0;i<width;i++) {
  for(int i=start;i<width;i++) {
#if 0
    Serial.print("setColumn1 addr=");
    Serial.print(addr);
    Serial.print(" col=");
    Serial.print(i);
    Serial.println();
#endif
    lc.setColumn(addr,i,_bitmap[i], false);
  }

  //右側の１列をクリア
  if (row < 0) {
    lc.setColumn(addr,start-1,0, false);
    return addr;
  }

  
  // 右側のデバイスへの表示
  int addrRight = addr + 1;
  if (addrRight >= maxDev) return addr;

  // 右側のデバイスの１列をクリア
  lc.setColumn(addrRight, width-1, 0, false);

  // 右側のデバイスに対する描画はなし
  if (width == 8) return addr;

  for(int i=0;i<mapsize;i++) {
    _bitmap[i] = bitmap[i];
    if (invert) _bitmap[i] = ~bitmap[i];
  }
  //下方向にスクロール
  for (int i=0;i<width;i++) {
    scrollBitmap(_bitmap, 8, 8, B1000);
  }

  for(int i=width;i<8;i++) {
#if 0
    Serial.print("setColumn2 addrRight=");
    Serial.print(addrRight);
    Serial.print(" col=");
    Serial.print(i);
    Serial.println();
#endif
    lc.setColumn(addrRight,i,_bitmap[i], false);
  }
 
  return addr;
}

void getBitmap(byte * bitmap, int code) {
  memcpy(bitmap, font8x8_basic[code], 8);
  //１ドット下にシフト
  for (int i=7;i>0;i--) {
    bitmap[i] = bitmap[i-1];
  }
  bitmap[0] = 0;
  //反時計90°回転
  rotateBitmap(bitmap, 8, 8, B01);
  //１ドット右にシフト
  for (int i=0;i<7;i++) {
    bitmap[i] = bitmap[i+1];
  }
  bitmap[7] = 0;
}


void setup() {
  Serial.begin(115200);
  Serial.println();

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  for (int addr=0;addr<numDevices;addr++) {
    lc.shutdown(addr,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(addr,8);
    /* and clear the display */
    lc.clearDisplay(addr);
  }

  // bitmap[numDevices+1][8]
  uint8_t bitmap[9][8];
  getBitmap(bitmap[0], 0x41);
  
  int startRow;
  int addr;
  unsigned long startMillis;
  unsigned long endMillis;
  unsigned long diffMillis;
  unsigned long delayMillis;

  
#if 1
  startRow = (numDevices * 8) - 1;
  while(1) {
    startMillis = millis();
    addr = showBitmap(bitmap[0], 8, startRow, numDevices, false);
    lc.pushAll();
    endMillis = millis();
    diffMillis = endMillis - startMillis;
    //Serial.println(diffMillis);
    if (addr < 0) break;
    startRow--;
    delayMillis = interval - diffMillis;
    if (delayMillis > 0) delay(delayMillis);
  }
  lc.clearDisplay(0);
  delay(5000);
#endif

#if 1
  startRow = (numDevices * 8) - 1;
  while(1) {
    startMillis = millis();
    addr = showBitmap(bitmap[0], 8, startRow, numDevices, true);
    lc.pushAll();
    endMillis = millis();
    diffMillis = endMillis - startMillis;
    //Serial.println(diffMillis);
    if (addr < 0) break;
    startRow--;
    delayMillis = interval - diffMillis;
    if (delayMillis > 0) delay(delayMillis);
  }
  lc.clearDisplay(0);
  delay(5000);
#endif


  char string[] = "Hello! World!";
  //char string[] = "Hello";
  int stringLen = strlen(string);
  int stringPos;
  for(stringPos=0;stringPos<(numDevices+1);stringPos++) {
    char ch = string[stringPos];
    getBitmap(bitmap[stringPos], ch);
  }


#if 1
  startRow = (numDevices * 8) - 1;
  //startRow = 8;
  while(1) {
    startMillis = millis();
#if 0
    showBitmap(bitmap[0], 8, startRow, numDevices, false);
    showBitmap(bitmap[1], 8, startRow+(8*1), numDevices, false);
    showBitmap(bitmap[2], 8, startRow+(8*2), numDevices, false);
    showBitmap(bitmap[3], 8, startRow+(8*3), numDevices, false);
    addr = showBitmap(bitmap[4], 8, startRow+(8*4), numDevices, false);
#else
    for(int index=0;index<numDevices;index++) {
      showBitmap(bitmap[index], 8, startRow+(8*index), numDevices, false);
    }
    addr = showBitmap(bitmap[numDevices], 8, startRow+(8*numDevices), numDevices, false);
#endif
    lc.pushAll();
    endMillis = millis();
    diffMillis = endMillis - startMillis;
    //Serial.println(diffMillis);
    //Serial.print("addr=");
    //Serial.println(addr);
    if (addr < 0) break;
    startRow--;
    if (startRow == -8 && stringPos < stringLen) {
#if 0
      memcpy(bitmap[0], bitmap[1], 8);
      memcpy(bitmap[1], bitmap[2], 8);
      memcpy(bitmap[2], bitmap[3], 8);
      memcpy(bitmap[3], bitmap[4], 8);
      //Serial.print("stringPos=");
      //Serial.println(stringPos);
      char ch = string[stringPos++];
      getBitmap(bitmap[4], ch);
#else
      for(int index=0;index<numDevices;index++) {
        memcpy(bitmap[index], bitmap[index+1], 8);
      }
      char ch = string[stringPos++];
      getBitmap(bitmap[numDevices], ch);
#endif
      startRow = 0;
    }
    delayMillis = interval - diffMillis;
    if (delayMillis > 0) delay(delayMillis);
  }
  lc.clearDisplay(0);
  delay(5000);
#endif

#if 1
  for(stringPos=0;stringPos<(numDevices+1);stringPos++) {
    char ch = string[stringPos];
    getBitmap(bitmap[stringPos], ch);
  }

  startRow = (numDevices * 8) - 1;
  //startRow = 8;
  while(1) {
    startMillis = millis();
    showBitmap(bitmap[0], 8, startRow, numDevices, true);
    showBitmap(bitmap[1], 8, startRow+(8*1), numDevices, true);
    showBitmap(bitmap[2], 8, startRow+(8*2), numDevices, true);
    showBitmap(bitmap[3], 8, startRow+(8*3), numDevices, true);
    addr = showBitmap(bitmap[4], 8, startRow+(8*4), numDevices, true);
    lc.pushAll();
    endMillis = millis();
    diffMillis = endMillis - startMillis;
    //Serial.print("addr=");
    //Serial.println(addr);
    if (addr < 0) break;
    startRow--;
    if (startRow == -8 && stringPos < stringLen) {
      memcpy(bitmap[0], bitmap[1], 8);
      memcpy(bitmap[1], bitmap[2], 8);
      memcpy(bitmap[2], bitmap[3], 8);
      memcpy(bitmap[3], bitmap[4], 8);
      //Serial.print("stringPos=");
      //Serial.println(stringPos);
      char ch = string[stringPos++];
      getBitmap(bitmap[4], ch);
      startRow = 0;
    }
    delayMillis = interval - diffMillis;
    if (delayMillis > 0) delay(delayMillis);
  }
  lc.clearDisplay(0);
  delay(5000);
#endif


}


void loop() {
}
