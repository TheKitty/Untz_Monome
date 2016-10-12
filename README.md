# Untz_Monome
Program an Adafruit Untz 8x8 or HellaUntz 8x16 to work as a monome.org controller via the mext protocol

This project uses the monome.org serial protocol defined at http://monome.org/docs/tech:serial which is used to control arrays of lighted buttons for creative control applications.

The Adafruit Untz is a monochrome (B/W) array of 8x8 or 8x16 keys. 
  ----> https://www.adafruit.com/products/1919
  ----> https://www.adafruit.com/products/1999
Using an Arduino Leonardo, the serial protocol is read and written according to the mext serial protocol.  
mext is used by the monome serialosc software which provides a host computer interface that may be used with 
many types of creative software packages such as Max MSP that interface well with monome compatible
software but the serial protocol may easily be handled by your choice of programming languages including
Processing, Python, C, Ruby, or others.

This work is open source per the LICENSE file.  

This is currently in development, if you try it and find issues, please place them in the Issues area for resolution.  Thank you.


To identify your untz as a mext-device to serialosc, you'll need to reflash the FTDI eeprom to appear as a monome device, using buspirate's 'piratereflash' or similar. 

-- os x /linux reflash instructions
Assuming you've got serialosc installed correctly, you should see the serialosc daemon running if you `ps auxw | grep serial`
To update your untznome's firmware, you'll need to: 
 
 - kill the `serialosc-detector` process
 - kill the `serialosc-device /dev/tty[...]` process

Then you can reflash your monome.  To get serialosc running again, kill the serialosc daemon process itself, and it will restart.
