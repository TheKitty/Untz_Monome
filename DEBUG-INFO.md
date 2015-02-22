# Untz_Monome Debugging  Updated 2/22/15
Program an Adafruit Untz 8x8 or HellaUntz 8x16 to work as a monome.org controller via the mext protocol

The serial protocol used in modern monome devices is called mext and is defined at http://monome.org/docs/tech:serial

The Untz Monome controller uses an Arduino Leonardo to communicate between the Adafruit Untz grid and a host computer running the serialosc software to interpret mext command protocols.

Debug mode

A debug mode has been added to the sketch.  This allows commands and results to be seen in the Arduino serial monitor.  Commands 0x00 to 0x1A to be malled to letters of the alphabet "a" to 
"{".  See the code for the letter corresponding to each command.  So entering the small letter "s" into the serial monitor clears all the LEDs (mext command byte 0x12).  Command "t" sets all the LEDs (mext 0x13) turns them all on.  This is good for testing things are communicating right.  All numeric values are entered as hexidecimal, with two letters equal to one 8 bit value, re. "00" is byte value 0.  "0F" is byte value 0x0F (decimal 15), etc.  So a command like mext 0x11 set LED (debug command "r"), to set LED at x, y of 3,3 would be entered "r0303".  Command "p" displays the serial number, etc.  ALl the corresponding command letters are listed in the Arduino code.

IF YOU WANT TO USE THIS CODE IN MAX OR OTHER SOFTWARE, comment out the #define DEBUG statement near the top of the code or else it will NOT work!!!!!!!!!!!!!!!

Assumptions

The documentation is not complete so some assumptions have been made.  If assumptions are wrong, please open a GitHub issue in the repository and these will be ironed out.

0) all indexes are zero based - so an 8x8 grid is numbered as LED 0 to 7 in both the x & y direction.

1) Serial communication - default set at 9600 baud which is probably wrong.  other programs describe differing baud rates.  The Arduinome software uses 57600 but states that it has been problematic (note no Arduinome software has been used in UntzMonome).

2) Grid orientation - assumed to be as bepow, this is a 50-50 shot but it seemed right
					USB
					PORT
y  x ->	0	1	2	3	4	5	6	7	(8... )
0
1
2
3
4
5
6
7

If the orientatiuon has y going left to right and x going top to bottom then the code will need to be changed accordingly or a rotate command of 90 degrees issued at the serialosc level by the control PC software.

3) The grid offset is read but not used for grids > 8x8 (for now).

4) The serial number returned is "a0000001" - true monome devices are currently like "m0000001" I believe.

5) Could be some other touches

Again if you note issues, log them in the GitHub issues area so they can be corrected.