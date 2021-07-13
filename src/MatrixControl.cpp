/*
 *    MatrixControl.cpp - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */


#include "MatrixControl.h"
#include <SPI.h>

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

#define BITBANG 1

MatrixControl::MatrixControl(int dataPin, int clkPin, int csPin, int numDevices) {
    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;
    if(numDevices<=0 || numDevices>8 )
        numDevices=8;
    maxDevices=numDevices;
    if (BITBANG) {
      pinMode(SPI_MOSI,OUTPUT);
      pinMode(SPI_CLK,OUTPUT);
    } else {
      SPI.begin();
    }
    pinMode(SPI_CS,OUTPUT);
    digitalWrite(SPI_CS,HIGH);
    SPI_MOSI=dataPin;
    for(int i=0;i<64;i++) 
        status[i]=0x00;
    for(int i=0;i<maxDevices;i++) {
        spiTransfer(i,OP_DISPLAYTEST,0);
        //scanlimit is set to max on startup
        setScanLimit(i,7);
        //decode is done in source
        spiTransfer(i,OP_DECODEMODE,0);
        clearDisplay(i);
        //we go into shutdown-mode on startup
        shutdown(i,true);
    }
}

int MatrixControl::getDeviceCount() {
    return maxDevices;
}

void MatrixControl::shutdown(int addr, bool b) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(b)
        spiTransfer(addr, OP_SHUTDOWN,0);
    else
        spiTransfer(addr, OP_SHUTDOWN,1);
}

void MatrixControl::setScanLimit(int addr, int limit) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(limit>=0 && limit<8)
        spiTransfer(addr, OP_SCANLIMIT,limit);
}

void MatrixControl::setIntensity(int addr, int intensity) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(intensity>=0 && intensity<16)	
        spiTransfer(addr, OP_INTENSITY,intensity);
}

void MatrixControl::clearAllDisplay(void) {
    for(int addr=0;addr<maxDevices;addr++) {
      clearDisplay(addr);
    }
}

void MatrixControl::clearDisplay(int addr) {
    int offset;

    if(addr<0 || addr>=maxDevices)
        return;
    //offset=addr*8;
    for(int i=0;i<8;i++) {
        status[i*8+addr]=0;
        spiTransfer(addr, i+1, status[i*8+addr]);
    }
}

void MatrixControl::setLed(int addr, int row, int column, boolean state, boolean output) {
    int offset;
    byte val=0x00;

    if(addr<0 || addr>=maxDevices)
        return;
    if(row<0 || row>7 || column<0 || column>7)
        return;
    offset=row*8;
    val=B10000000 >> column;
    if(state)
        status[offset+addr]=status[offset+addr]|val;
    else {
        val=~val;
        status[offset+addr]=status[offset+addr]&val;
    }
    if (output) spiTransfer(addr, row+1, status[offset+addr]);
}

void MatrixControl::pushRow(int addr, int row) {
    int offset;
    if(addr<0 || addr>=maxDevices)
        return;
    if(row<0 || row>7)
        return;
    offset=row*8;
    spiTransfer(addr, row+1, status[offset+addr]);
}

void MatrixControl::pushAllRow(int row) {
    int offset;
    if(row<0 || row>7)
        return;
    offset=row*8;
    //spiTransfer(0, row+1, status[offset+0]);
    //spiTransfer(1, row+1, status[offset+1]);
    //spiTransfer(2, row+1, status[offset+2]);
    //spiTransfer(3, row+1, status[offset+3]);
#if 0
    Serial.print("status=");
    for(int i=0;i<4;i++) {
      Serial.print(" ");
      Serial.print(status[offset+i], HEX);
    }
    Serial.println();
#endif
    spiTransfers(row+1, &status[offset]);
}

void MatrixControl::pushAll(void) {
    for(int row=0;row<8;row++) {
      pushAllRow(row);
    }
}

void MatrixControl::setRow(int addr, int row, byte value, boolean output) {
    int offset;
    if(addr<0 || addr>=maxDevices)
        return;
    if(row<0 || row>7)
        return;
    offset=row*8;
    status[offset+addr]=value;
    if (output) spiTransfer(addr, row+1, status[offset+addr]);
}

void MatrixControl::setColumn(int addr, int col, byte value, boolean output) {
    byte val;

    if(addr<0 || addr>=maxDevices)
        return;
    if(col<0 || col>7) 
        return;
    for(int row=0;row<8;row++) {
        val=value >> (7-row);
        val=val & 0x01;
        setLed(addr, row, col, val, output);
    }
}

#if 0
void MatrixControl::setDigit(int addr, int digit, byte value, boolean dp) {
    int offset;
    byte v;

    if(addr<0 || addr>=maxDevices)
        return;
    if(digit<0 || digit>7 || value>15)
        return;
    offset=addr*8;
    v=pgm_read_byte_near(charTable + value); 
    if(dp)
        v|=B10000000;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);
}

void MatrixControl::setChar(int addr, int digit, char value, boolean dp) {
    int offset;
    byte index,v;

    if(addr<0 || addr>=maxDevices)
        return;
    if(digit<0 || digit>7)
        return;
    offset=addr*8;
    index=(byte)value;
    if(index >127) {
        //no defined beyond index 127, so we use the space char
        index=32;
    }
    v=pgm_read_byte_near(charTable + index); 
    if(dp)
        v|=B10000000;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);
}
#endif

void MatrixControl::spiTransfer(int addr, volatile byte opcode, volatile byte data) {
    //Create an array with the data to shift out
    int offset=addr*2;
    int maxbytes=maxDevices*2;

    for(int i=0;i<maxbytes;i++)
        spidata[i]=(byte)0;
    //put our device data into the array
    spidata[offset+1]=opcode;
    spidata[offset]=data;
    //enable the line 
    digitalWrite(SPI_CS,LOW);
    //Now shift out the data 
//Serial.print("spidata: addr=");
//Serial.print(addr);
    for(int i=maxbytes;i>0;i--) {
      if (BITBANG) {
        shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]);
      } else {
        SPI.transfer(spidata[i-1]);
      }
//Serial.print(" 0x");
//Serial.print(spidata[i-1], HEX);
    } // end for
    //latch the data onto the display
    digitalWrite(SPI_CS,HIGH);
//Serial.println();
}    

void MatrixControl::spiTransfers(volatile byte opcode, volatile byte *data) {
    //Create an array with the data to shift out
    int maxbytes=maxDevices*2;

    for(int i=maxDevices;i>0;i--) {
        spidata[i*2-1]=opcode;
        spidata[i*2-2]=data[i-1];
        //Serial.println(data[i-1], HEX);
    }
    //put our device data into the array
    //enable the line 
    digitalWrite(SPI_CS,LOW);
    //Now shift out the data 
//Serial.print("spidata:");
    for(int i=maxbytes;i>0;i--) {
      if (BITBANG) {
        shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]);
      } else {
        SPI.transfer(spidata[i-1]);
      }
//Serial.print(" 0x");
//Serial.print(spidata[i-1], HEX);
    } // end for
    //latch the data onto the display
    digitalWrite(SPI_CS,HIGH);
//Serial.println();
}    

