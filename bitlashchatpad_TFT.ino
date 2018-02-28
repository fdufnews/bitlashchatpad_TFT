/***
	bitlashchatpad.ino:	Bitlash with an Xbox Chatpad as input interface

	Copyright (C) 2008-2013 Bill Roy

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

  11/2016 fdufnews Xbox Chatpad interface. Uses a modified chatpad which sends chars over serial @4800bauds
   This application uses a Xbox Chatpad as an input keyboard.
   The aim of this is to have a small portable device.

  11/2017 fdufnews added support of an OPENSMART-breakout board for a 3.0" ILLI9326 TFT screen

  02/2018 fdufnews 
          added support of special chars (backspace, bell, newline)
          added blinking cursor, line wrapping management and font scale
          added user function textsize, cls, textcolor

***/
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define serialPrintByte(b) Serial.write(b)
//	#include <util.h>
#else
#include "WProgram.h"
#define serialPrintByte(b) Serial.print(b,BYTE)
#endif
#include "bitlash.h"

// TFT display support of OPEN-SMART-breakout board for 3.0" ILLI9326
// includes and defines related to the TFT screen
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // hardware layer
#include "define.h"          // define some constants related to the screen

MCUFRIEND_kbv tft;           // instance of a screen object
uint8_t textSize=1;          // text scale factor for TFT screen
uint16_t red=255,green=255,blue=255;      // text color

////////////////////////////////////////
//
//	Declare function(s) associated to Chatpad
//
////////////////////////////////////////

/* function called to read chatpad
 * manage a blinking cursor
 * if any char is read, send it to doCharacter (interpreter input)
 * 
 * This function shall be called as frequently as possible so:
 *     the cursor blinks at constant period
 *     there is no latency in keyboard reading
*/

void getKey(void) {
  int x = tft.getCursorX();             //get cursor position
  int y = tft.getCursorY();
  static boolean state=false;           // save current state of cursor
  static unsigned long lastBlink=0;     // save time we change state of cursor
  // blinking cursor
  if (millis()-lastBlink>CURSOR_BLINK_PERIOD){ // time ellapsed
    state=!state;                       // change state
    lastBlink=millis();                 // save time the state changed
    if (state){                         // based on current state draw an underscore or a space
      tft.write('_');
    }else{
      tft.fillRect(x, y, FONT_SIZE_X*textSize, FONT_SIZE_Y*textSize, BLACK);  // draw a space at current cursor position
    }
    tft.setCursor(x, y);                // set cursor position back to its current position
  }
  if (Serial1.available()) {  // if key pressed
    char a = Serial1.read();
    if (a != 0) {
      lastBlink=millis();               // reset blink time
      tft.fillRect(x, y, FONT_SIZE_X*textSize, FONT_SIZE_Y*textSize, BLACK);  // put a space to erase the underscore if present
      doCharacter(a);                   // Send character to interpreter
    }
  }
}

////////////////////////////////////////
//
//  Declare function(s) associated to TFT
//
////////////////////////////////////////

/*
 *  Handler to redirect output interface
 *  Here we manage printing on the TFT
 *
 * This function is not directly called by the application but by bitlash printing engine
*/
void TFTHandler(byte b) {
  /*  Prints incoming characters on the TFT screen
   *  
   *  Before printing, see if any special char first
   * 
   * line wrapping : before printing, test if cursor x position is outside the screen. If so move to the next line
   * bell      (0x07)    : silent bell, blinks screen (normal, invert) 4 times
   * backspace (0x08)    : move the cursor one character back, draw a space and return
   * newline   (0x0A)    : move the cursor to the next line and erase the line (and the following one).
   *                       If cursor y position is outside the screen rolls back to top
  */
  int x = tft.getCursorX();
  int y = tft.getCursorY();
  char newline = x >= (tft.width() - FONT_SIZE_X*textSize);
  if (b == 7){
    unsigned int reg= tft.readReg(0x401);
    for (char i=0;i<8;i++){
      tft.WriteCmdData(0x401,reg=reg^1);
      delay(250);
    }
  } else if(b == 8) {
    tft.fillRect(x - FONT_SIZE_X*textSize, y, FONT_SIZE_X*textSize, FONT_SIZE_Y*textSize, BLACK); // erase char
    tft.setCursor(x - FONT_SIZE_X*textSize, y); // put cursor back one place
  } else if (b == 0x0a || newline) {
    if (y >= (tft.height() - FONT_SIZE_Y*textSize)) {
      y = 0;
    } else {
      y += FONT_SIZE_Y*textSize;
    }
    tft.fillRect(0, y, tft.width(), 16, BLACK); // erase line
    tft.setCursor(0, y); // put cursor at beginning of new line
    if (newline) {
      tft.write(b);
    }
  } else {
    tft.write(b);
  }
}

/*
 *  splashscreen
 * 
 *  Print Bitlash with version and release numbers on a blue background
*/
void splashscreen(void) {
  tft.fillScreen(LBLUE);
  tft.setTextSize(5);
  tft.setTextColor(LRED);
  tft.setCursor(20, 75);
  doCommand((char*)"printf(\"BITLASH V%d.%d\",version,release)");
  tft.setTextColor(RED);
  tft.setCursor(23, 78);
  doCommand((char*)"printf(\"BITLASH V%d.%d\",version,release)");
  delay(3000);
}

////////////////////////////////////////
//
//  Declare user function(s)
//
////////////////////////////////////////

// funcTextSize
// use given argument to set text scale
// if no given argument returns current scale
//
numvar funcTextSize(void){
  if (getarg(0)>0){
      textSize = getarg(1);
      tft.setTextSize(textSize);
  }
  return(textSize);
  }

// funcTextcolor
// use given arguments to set text color
// if 1 argument, it is supposed to be a 16 bits value coding colors in 5-6-5 format
// if 3 arguments, they are supposed to hold R-G-B on 3 8 bits value
// in all other cases returns color actually used on a 16 bits word coding colors in 5-6-5 format
//
numvar funcTextcolor(void){
  if (getarg(0)==1){            // extracts the 3 colors from the 16 bits word
    red = (getarg(1) & 0xF800) >>8;
    green = (getarg(1) & 0x7E0)>>3;
    blue = (getarg(1) & 0x1F)<<3;
    tft.setTextColor((uint16_t) getarg(1));
  }else if (getarg(0)==3){      // updates the colors with the 3 arguments
    red = (uint8_t) getarg(1);
    green = (uint8_t) getarg(2);
    blue = (uint8_t) getarg(3);
    tft.setTextColor(((red & 0xF8)<<8) | ((green & 0xFC)<<3) | (blue >> 3));
  }
  return(((red & 0xF8)<<8) | ((green & 0xFC)<<3) | (blue >> 3)); // returns current color
}

// funcCls
// Clear screen
//
numvar funcCls(void){
  tft.fillScreen(BLACK);
  tft.setCursor(0,0);
  return(0);
}

/*
 *  setup
*
 *  init Serial1 to handle chatpad
 *  add hook to display manager
 *  init TFT screen
 *  init bitlash
 *  add user functions to bitlash dictionnary
 *  display splashscreen
 *  display banner
*/
void setup(void) {
  uint16_t g_identifier;       // screen ID

  Serial1.begin(4800);         // chatpad connected on Serial1

  g_identifier = tft.readID();
  tft.begin(g_identifier);
  
  setOutputHandler(&TFTHandler); // hook for TFT screen handler
  initBitlash(57600);
  
  addBitlashFunction("textsize", (bitlash_function)funcTextSize);
  addBitlashFunction("cls", (bitlash_function)funcCls);
  addBitlashFunction("textcolor", (bitlash_function)funcTextcolor);
  tft.setRotation(3);
  splashscreen();
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  tft.setTextSize(1);
  doCommand((char*)"banner");
  doCommand("printf(\"> \")");
}

/*
   main loop
   - poll chatpad
   - call Bitlash background manager
*/
void loop(void) {

  getKey();  // poll chatpad
  runBitlash();
}
