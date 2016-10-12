/***********************************************************
  This program takes an Adafruit 8x8 UNTX or 16x8 HellaUNTZ 
  controller and makes it compatible with Monome via the mext protocol
  
  4 (UNTZ) or 8 (Hella UNTZ) Adafruit Trellis tactile LED 
  matrices can be used.  #define NUMTRELLIS to the number in use.

  Designed specifically to work with the Adafruit UNTZ and HellaUNTZ
  ----> https://www.adafruit.com/products/1919
  ----> https://www.adafruit.com/products/1999

  Trellis code written by Limor Fried/Ladyada for Adafruit Industries.  
  UNTZ key code by Phil Burgess for Adafruit Industries
  Monome emulation written by Mike Barela for Adafruit Industries
  MIT license, all text above must be included in any redistribution
  
  Version 1.0  2015-02-28  First Version
 ***********************************************************/
#include <Wire.h>                  // Uses I2C communication Leonardo to Trellis
#include "Adafruit_Trellis.h"      // Trellis driver library
//#include "Adafruit_UNTZtrument.h"  // UNTZtrument library for button mapping
                                     //   currently button map code copied, change if encoders added
// If you have the next line active, it allows you to enter values into the serial terminal in ASCII
//   rather than communicate via binary mext commands.  Debug does not output binary return values
// Comment out next line to interface with monome software
// #define DEBUG 1

// **** SET # OF TRELLISES HERE:  4 = UNTZ, 8 = HellaUNTZ, don't use other values
#define NUMTRELLIS 8              

#define INTERVAL 12L // interval between scans

Adafruit_Trellis matrix[NUMTRELLIS] = {    // Instance matrix using number of Trellises set above
  Adafruit_Trellis(), Adafruit_Trellis(),
  Adafruit_Trellis(), Adafruit_Trellis()
#if NUMTRELLIS > 4
 ,Adafruit_Trellis(), Adafruit_Trellis(),
  Adafruit_Trellis(), Adafruit_Trellis()
#endif
};

Adafruit_TrellisSet trellis = Adafruit_TrellisSet(
  &matrix[0], &matrix[1], &matrix[2], &matrix[3]
#if NUMTRELLIS > 4
 ,&matrix[4], &matrix[5], &matrix[6], &matrix[7]
#endif
);

#define numKeys (NUMTRELLIS * 16)          // Number of keys in the grid (will be 84 or 128 only)

const uint8_t gridNumber = 0x01;           // This code is for Grid #2 (change for > 1 grid)
const uint8_t gridX    = (NUMTRELLIS*2);   // Will be either 8 or 16 for Untz
const uint8_t gridY    = 8;                // standard for both UNTZs
const uint8_t numGrids = (NUMTRELLIS / 4); //
uint8_t       offsetX  = 0;                // offset for HellaUNTZ only (8x8 UNTZ can't offset)
unsigned long prevReadTime = 0L;           // Keypad polling timer

String deviceID  = "monome                          ";  // 32
String serialNum = "m1000009"; // 8
struct coord { 
   uint8_t x; 
   uint8_t y; } coord;      // x/y coordinates for a single point

void setup() {
  Serial.begin(115200);     // check baud rate for mext
  
  trellis.begin(            // Initialize trellises in UNTZ
    0x70, 0x71, 0x72, 0x73
#if NUMTRELLIS > 4
   ,0x74, 0x75, 0x76, 0x77
#endif
   );
   
   trellis.clear();              // signal the program is running
   trellis.writeDisplay();
   for(uint8_t i=0; i<3; i++) {
     trellis.setLED(0);
     trellis.setLED(numKeys-1);
     trellis.writeDisplay();
     delay(250);
     trellis.clrLED(0);
     trellis.clrLED(numKeys-1);
     trellis.writeDisplay();
   }
}

void loop() {
  unsigned long t = millis();
  if (Serial.available() > 0) {            // there is a remote command waiting
      do {
         processSerial();
      } while(Serial.available() > 16);
  }
  // poll keyboard state and report if something change.  This will be key pressed or key released
  if( (unsigned long)(t - prevReadTime) >= INTERVAL) {         // 20ms = min Trellis poll time
    if (trellis.readSwitches() > 0) {
      trellisKeys();
    }
  }
  prevReadTime = t;
  delay(11);             // shouldn't be needed but it is required to have it work...for now
}

void processSerial() {
  uint8_t identifierSent;    // Command byte sent from controller to matrix
//  uint8_t sectionSent;
//  uint8_t commandSent;
  uint8_t deviceAddress;     // device address sent from controller
  uint8_t dummy;             // for reading in data not used by the matrix
  uint8_t intensity = 255;   // LED intensity.  Since UNTZ is on or off, 0=off, >0=on, initially on
  uint8_t readX, readY;      // X and Y values read from driver
  uint8_t i;                 // loop variable
  uint8_t index;             // Linear index of the LED being worked with (0-63 or 0-127)

  // Get Command Identifier
  // first byte of packet is identifier in the form: [(a << 4) + b]
  // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
  // b = command (ie. query, enable, led, key, frame)
  identifierSent = Serial.read();
#if DEBUG
  // Map the keys a to z and { to the cbinary commands sent to matrix for debug in serial window
  if(identifierSent >='A' and identifierSent <= '_') identifierSent += 32; // lower case to upper case (in case)
  if(identifierSent >= 'a') {                                  // a = 0x00, n= 0x01 etc. in serial monitor
       identifierSent -= 'a';   // So commands 0x00 to 0
    }
#endif  
//  commandSent = identifierSent & 0x0F;         // lower 4 bits,  implicitly done in case statement
//  sectionSent = (identifierSent >> 4) & 0x0F;  // upper 4 bits
  
  switch( identifierSent ) {  
    // 0x00-0x0f are system commands
    case 0x00: writeInt((uint8_t)0x00);       // System/Query response 0x00 -> 0x00 (debug "a")
               writeInt(numGrids);            // could be wrong
               writeInt((uint8_t)0x01);       // Always one enclosure
//               Serial.write((uint8_t)0x00); // System/Query 1 LED grid - may not be needed
//               Serial.write(led_grid);      //   but documentation is not clear
//               Serial.write((uint8_t)0x01);  
               break;
    case 0x01: writeInt((uint8_t)0x01);      // system / query ID /sys/id (debug "b")
//               for(i=0; i<wcslen(deviceID); i++) {
                 Serial.print(deviceID);  // write out device ID (32 characters)
//               }
               break;
    case 0x02: for(i=0; i<32; i++) {          // system / write ID  (debug "c")
                 deviceID[i] = Serial.read(); // Get ID into grid program
               }
               break;  
    case 0x03: writeInt((uint8_t)0x02);  // system / request grid offsets (debug "d")
               writeInt(offsetX);        //  0 for UNTZ, could be 0 or 8 for HellaUNTZ
               writeInt((uint8_t)0x00);  // UNTZ and HellaUNTZ are only 8 in Y  
               break;
    case 0x04: dummy = readInt();        // system / set grid offset  (debug "e")
               readX = readInt();        // An offset of 8 is valid only for HellaUNTZ
               readY = readInt();        // An offset is invalid for Y as it's only 8 
               if(numKeys > 64 && readX == 8) offsetX = 8; // not sure how to use this well
               break;
    case 0x05: writeInt((uint8_t)0x03);   // system / request grid size  /sys/size (debug "f")
               writeInt(gridX);
               writeInt(gridY); 
               break; 
    case 0x06: readX = readInt();         // system / set grid size (debug "g")
               readY = readInt();         // currently this does nothing.  Get more info? 
               break;
    case 0x07: break;                     // system / get ADDR (scan) (we don't send I2C via serial) "h"
    case 0x08: deviceAddress = readInt(); // system / set ADDR (scan) (debug "i")
               dummy = readInt();         // address value
               break;
    case 0x0F: writeInt((uint8_t)0x0F);   // Send Serial Number (debug "p")
               Serial.print(serialNum);   // monome SN m####### or m64-#### m128-### m256-####
               break;
    //
    // 0x10-0x1F are for an LED Grid Control.  All bytes incoming, no responses back
    case 0x10: readX = readInt();  // assumes x is 0 to 7 or 0 to 15 (debug "q")
               readY = readInt();  // assumes y is 0 to 7
               trellis.clrLED(mapXYtoLinear(readX,readY));
               break;
    case 0x11: readX = readInt();  // assumes x is 0 to 7 or 0 to 15 (debug "r") 
               readY = readInt();  // assumes y is 0 to 7
               trellis.setLED(mapXYtoLinear(readX,readY));
               break;
    case 0x12: trellis.clear();  // /prefix/led/all 0  (need to send back a 0x12?) (debug "s")
               break;
    case 0x13: for (i=0; i<numKeys; i++) {    // /prefix/led/all/1   (debug "t")
                 trellis.setLED(i);           //   all keys turn on
	       }
               break;
    case 0x14: readX = readInt();                    // set grid (debug "u")
               readY = readInt();  
               readX >> 3; readX << 3; // floor to multiple of 8
               readY >> 3; readY << 3;
               if(readX == 8 && numKeys > 64) break; // trying to set an 8x16 grid on a pad with only 64 keys
               if(readY != 0) break;                 // since we only have 8 LEDs in a column, no offset for UNTZs
               for(uint8_t y=0; y<8; y++) {          // each i will be a row
                  intensity = readInt();             // read one byte of 8 bits on/off
                  for(int8_t x=0; x<8; x++) {        // for 8 LEDs on a row
                    index = mapXYtoLinear(readX + x, y); // get linear LED value                  
                    if( (intensity>>x) & 0x01 ) {    // set LED if the intensity bit is set, clear otherwise
                      trellis.setLED(index);
                    }
                    else {
                      trellis.clrLED(index);
                    }
                 } // end for x
               } // end for y
               break;
    case 0x15: readX = readInt();              // led-grid / set row (debug "v")
               readX >> 3; readX << 3;
               readY = readInt();              // may be any value
               intensity = readInt();          // read one byte of 8 bits on/off
               for(i=0; i<8; i++) {            // for the next 8 lights in row
                  index = mapXYtoLinear(readX+i, readY);
                  if((intensity>>i) & 0x01) {  // if intensity bit set, light
                     trellis.setLED(index);
                  }
                  else {
                     trellis.clrLED(index);    // if bit clear, turn off              
                  }
               }
               break;
    case 0x16: readX = readInt();                   // led-grid / column set (debug "w")
               readY = readInt(); 
               readY >> 3 ; readY << 3; // floor to multiple of 8
               intensity = readInt();              // read one byte of 8 bits on/off
               if(readY != 0) break;               // Untz' only have 8 lights in a column
               for(i=0; i<8; i++) {                // for the next 8 lights in column
                  index = mapXYtoLinear(readX, i); // get index at the x, and for each y
                  if((intensity>>i) & 0x01) {      // if intensity bit set, light
                     trellis.setLED(index);
                  }
                  else {
                     trellis.clrLED(index);        // if bit clear, turn off              
                  }
               }
               break;
    case 0x17: intensity = readInt();                    // led-grid / intensity (debug "x")
               for (i=0; i<numKeys; i++) {               // check all keys
                  if(trellis.isLED(i)) {                 // if LED is on
                    if(intensity==0) trellis.clrLED(i);  // turn off if intensity=0
                  }                                      // otherwise leave alone
	       }
               trellis.setBrightness(intensity);
               trellis.writeDisplay();
               break;
    case 0x18: readX = readInt();                   // led-grid / set LED intensity (debug "y")
               readY = readInt();                   // read the x and y coordinates
               intensity = readInt();               // read the intensity value (0-255, 0x00-0xFF)
               index = mapXYtoLinear(readX,readY);
               if(intensity) {                      // Since Untz is single intensity, if intensity > 0
                 trellis.setLED(index);             //   set the pixel
               }
               else {
                 trellis.clrLED(index);             //   otherwise clear the pixel
               }
               break;
    case 0x19: // set all leds 
           intensity = readInt();       // led-grid / set all LEDs to intensity (debug "z")
             for (i=0; i<numKeys; i++) {  // check all keys
               if(intensity > 0) 
                  trellis.setLED(i);      // turn all on (any positive intensity is full on)
               else
                  trellis.clrLED(i);      // turn off if intensity = 0
	       }
           break;
    case 0x1A:  // set 8x8 block.
           readX = readInt();               // led-grid / map intensity (debug "{")
           readX << 3; readX >> 3;
           readY = readInt();  
           readY << 3; readY >> 3;
           for(uint8_t y=0; y<8; y++) {
             for( uint8_t x=0; x<8; x++){
               index = mapXYtoLinear(readX + x, y);
               if(index > numKeys) break;
               if((x+y)%2 == 0) {           // even bytes, read intensity, use upper nybble
                  intensity = readInt();
                  if(intensity>>4 & 0x0F ) 
                    trellis.setLED(index);  // turn all on (any positive intensity is full on)
                  else
                    trellis.clrLED(index);  // turn off if intensity=0 
               }                      
               else {                       // odd bytes, use lower nybble
                  if(intensity & 0x0F ) 
                    trellis.setLED(index);  // turn all on (any positive intensity is full on)
                  else
                    trellis.clrLED(index);  // turn off if intensity=0 
               }
             }
           }
           break;
    case 0x1B:  //set 8x1 row of intensities
             readX = readInt();               // led-grid / map intensity (debug "{")
             readX << 3; readX >> 3;
             readY = readInt();  
             for(uint8_t x = 0; x < 8; x++) {
                 intensity = readInt();
                 if (intensity) { 
                     trellis.setLED(mapXYtoLinear(readX +x,  readY));
                     }
             }
             break;
    case 0x1C:  // set 1x8 column by intensity; we can't, so any nonzero one is displayed
             readX = readInt();               // led-grid / map intensity (debug "{")
             readY = readInt();  
             readY << 3; readY >> 3;
             for(uint8_t y = 0; y < 8; y++) {
                 intensity = readInt();
                 if (intensity) { 
                     trellis.setLED(mapXYtoLinear(readX,  readY+y));
                     }
             }
             break;
    break;
    // 0x20-0x2f are for a Key Grid    
    //   These are handled in the main loop by function pollKeys
    // 0x3x are digital out,  0x4x are digital line in,  0x5x are encoder
    // 0x6x are analog in, 0x70 are analog out
    // 0x80 are tilt
    // 0x90 variable 64 LED ring 

    default:   break;             // Any other incoming commands do nothing        
  }
  if(identifierSent>>4 == 0x01)   // write changes to Trellis for set LED functions
     trellis.writeDisplay();      //   eliminates multiple calls in switch statement
  return;
}  

void writeInt(uint8_t value) {    // Write an 8 bit value out to controlling PC
#if DEBUG
   Serial.print(value,HEX);       // For debug, values are written to the serial monitor in Hexidecimal
   Serial.print(" ");
#else
   Serial.write(value);           // standard is to write out the 8 bit value on serial
#endif
}

int8_t readInt() {                // Read an 8 bit value from serial port (0x00 to 0xFF)
#if DEBUG
  uint8_t number;                 // for debug, you enter two hex characters for each 8 bit unsigned int
  char value;
  Serial.print("Characters: ");
  value = Serial.read();
  Serial.print(value);
  Serial.print(" and ");
  if(value >='0' && value <= '9') number=value-'0';
  if(value >='A' && value <= 'F') number=value+10-'A';
  if(value >='a' && value <= 'f') number=value+10-'a';
  number = number<<4;
  value = Serial.read();
  Serial.print(value);
  if(value >='0' && value <= '9') number=number+value-'0';
  if(value >='A' && value <= 'F') number=number+value+10-'A';
  if(value >='a' && value <= 'f') number=number+value+10-'a';
  Serial.print(" is ");
  Serial.print(number, HEX);
  Serial.println(" hex");
  return(number);
#else
  return(Serial.read());          // for non-debug, return one 8 bit value unsigned 0x00 to 0xFF
#endif
}
  
// Function TrellisKeys checks the switch matrix to see if any key has been pressed and/or released 
//   and returns that status to the mext interface via serial
void trellisKeys() {
     // go through every button
     for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed, tell controller
	if (trellis.justPressed(i)) {
          mapLinearToXY(i);
#if DEBUG   
          Serial.print("Press at");
          Serial.print(coord.x);
          Serial.print(",");
          Serial.println(coord.y);
#else
          Serial.write(0x20); 
          Serial.write(coord.x);
	  Serial.write(coord.y);
#endif
          return;
	} 
	// if it was released, tell controller
	if (trellis.justReleased(i)) {
          mapLinearToXY(i);
#if DEBUG
          Serial.print("Release at");
          Serial.print(coord.x);
          Serial.print(",");
          Serial.println(coord.y);          
#else
          Serial.write(0x21); 
          Serial.write(coord.x);
	  Serial.write(coord.y);
#endif
	}
     } // end for
}

// convert single coordinate value to x, y values for either UNTZ or HellaUNTZ

void mapLinearToXY(uint8_t value) {  
    i2xy(value, &coord.x, &coord.y);
} 

// convert x, y value to linear value
uint8_t mapXYtoLinear( uint8_t x, uint8_t y ) {
    return(xy2i(x, y));
}

// These functions are from Adafruit_UNTZtrument.h - transplanted here to keep code size low
// If your code uses encoders, go ahead and use the UNTZtrument library
// Lookup tables take some PROGMEM size but they make for fast constant-time lookup.
static const uint8_t PROGMEM
  i2xy64[] = { // Remap 8x8 TrellisSet button index to column/row
    0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31,
    0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33,
    0x40, 0x50, 0x60, 0x70, 0x41, 0x51, 0x61, 0x71,
    0x42, 0x52, 0x62, 0x72, 0x43, 0x53, 0x63, 0x73,
    0x04, 0x14, 0x24, 0x34, 0x05, 0x15, 0x25, 0x35,
    0x06, 0x16, 0x26, 0x36, 0x07, 0x17, 0x27, 0x37,
    0x44, 0x54, 0x64, 0x74, 0x45, 0x55, 0x65, 0x75,
    0x46, 0x56, 0x66, 0x76, 0x47, 0x57, 0x67, 0x77 },
  i2xy128[] = { // Remap 16x8 TrellisSet button index to column/row
    0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31,
    0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33,
    0x40, 0x50, 0x60, 0x70, 0x41, 0x51, 0x61, 0x71,
    0x42, 0x52, 0x62, 0x72, 0x43, 0x53, 0x63, 0x73,
    0x80, 0x90, 0xA0, 0xB0, 0x81, 0x91, 0xA1, 0xB1,
    0x82, 0x92, 0xA2, 0xB2, 0x83, 0x93, 0xA3, 0xB3,
    0xC0, 0xD0, 0xE0, 0xF0, 0xC1, 0xD1, 0xE1, 0xF1,
    0xC2, 0xD2, 0xE2, 0xF2, 0xC3, 0xD3, 0xE3, 0xF3,
    0x04, 0x14, 0x24, 0x34, 0x05, 0x15, 0x25, 0x35,
    0x06, 0x16, 0x26, 0x36, 0x07, 0x17, 0x27, 0x37,
    0x44, 0x54, 0x64, 0x74, 0x45, 0x55, 0x65, 0x75,
    0x46, 0x56, 0x66, 0x76, 0x47, 0x57, 0x67, 0x77,
    0x84, 0x94, 0xA4, 0xB4, 0x85, 0x95, 0xA5, 0xB5,
    0x86, 0x96, 0xA6, 0xB6, 0x87, 0x97, 0xA7, 0xB7,
    0xC4, 0xD4, 0xE4, 0xF4, 0xC5, 0xD5, 0xE5, 0xF5,
    0xC6, 0xD6, 0xE6, 0xF6, 0xC7, 0xD7, 0xE7, 0xF7 },
  xy2i64[8][8] = { // Remap [row][col] to Trellis button/LED index
    {  0,  1,  2,  3, 16, 17, 18, 19 },
    {  4,  5,  6,  7, 20, 21, 22, 23 },
    {  8,  9, 10, 11, 24, 25, 26, 27 },
    { 12, 13, 14, 15, 28, 29, 30, 31 },
    { 32, 33, 34, 35, 48, 49, 50, 51 },
    { 36, 37, 38, 39, 52, 53, 54, 55 },
    { 40, 41, 42, 43, 56, 57, 58, 59 },
    { 44, 45, 46, 47, 60, 61, 62, 63 } },
  xy2i128[8][16] = {
    {  0,  1,  2,  3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51 },
    {  4,  5,  6,  7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55 },
    {  8,  9, 10, 11, 24, 25, 26, 27, 40, 41, 42, 43, 56, 57, 58, 59 },
    { 12, 13, 14, 15, 28, 29, 30, 31, 44, 45, 46, 47, 60, 61, 62, 63 },
    { 64, 65, 66, 67, 80, 81, 82, 83, 96, 97, 98, 99,112,113,114,115 },
    { 68, 69, 70, 71, 84, 85, 86, 87,100,101,102,103,116,117,118,119 },
    { 72, 73, 74, 75, 88, 89, 90, 91,104,105,106,107,120,121,122,123 },
    { 76, 77, 78, 79, 92, 93, 94, 95,108,109,110,111,124,125,126,127 } };

uint8_t xy2i(uint8_t x, uint8_t y) {
	if(y > 7) return 255;
	if(numKeys == 64) {
		if(x > 7) return 255;
		return pgm_read_byte(&xy2i64[y][x]);
	} else {
		if(x > 15) return 255;
		return pgm_read_byte(&xy2i128[y][x]);
	}
}

void i2xy(uint8_t i, uint8_t *x, uint8_t *y) {
	if(i > numKeys) {
		*x = *y = 255;
		return;
	}
	uint8_t xy = pgm_read_byte((numKeys == 64) ? &i2xy64[i] : &i2xy128[i]);
	*x = xy >> 4;
	*y = xy & 15;
}

