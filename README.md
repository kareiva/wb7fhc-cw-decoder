   WB7FHC's Morse Code Decoder v. 1.1
   (c) 2014, Budd Churchward - WB7FHC
   This is an Open Source Project
   http://opensource.org/licenses/MIT
   
   Search YouTube for 'WB7FHC' to see several videos of this project
   as it was developed.
   
   MIT license, all text above must be included in any redistribution
   **********************************************************************
   
   This project makes use a custom built tone decoder module using
   the LM567C microchip. Details of this module will eventually be posted
   on line. This module allows you to tune to the frequency of a specific
   tone while ignoring noice and other tones of different frequencies
   
   The program will automatically adjust to the speed of code that
   is being sent. The first few characters may come out wrong while it
   homes in on the speed. If you are not seeing solid copy, press the
   restart button on your Arduino. You can try adjusting the tone decoder.
   Lowering the volume of the incoming CW can also help. If the tone decoder
   is not centered on the frequency of the incomming signal, you may have
   to fine tune the module as you lower the volume.
   
   The software tracks the speed of the sender's dahs to make
   its adjustments. The more dahs you send at the beginning
   the sooner it locks into solid copy.
   
   After a reset, the following text is very difficult to lock in on:
   'SHE IS HIS SISTER' because there are only two dahs in the whole
   phrase and they come near the end. However, if you reset and then
   send 'CALL ME WOODY' it will match your speed quite quickly.
   
   This project is built around the 20x4 LCD display. The sketch includes
   funtions for word wrap and scrolling. If a word extends beyond the 20
   column line, it will drop down to the next line. When the bottom line
   is filled, all lines will scroll up one row and new text will continue
   to appear at the bottom.
   
   This version makes use of the 4 digit parallel method of driving the
   display.
   
   If you are planning on using a 16x2 you will want to make some changes.
   Frankly, I don't think scrolling makes sense with only two lines.
   Sometimes long words or missed spaces will result in only two words
   left on your display. If you don't have a 20x4 (they're really only a
   few bucks more) you might want to leave out the word wrap and scrolling.
   
   Hook up your LCD panel to the Arduino using these pins:
     LCD pin  1 to GND
     LCD pin  2 to +5V
     LCD pin  4 to D7
     LCD pin  6 to D6
     LCD pin 11 to D5
     LCD pin 12 to D4
     LCD pin 13 to D3
     LCD pin 14 to D2
     LCD pin 15 to +5V
     LCD pin 16 to GND
     
  Data from pin 8 of the LM567C will be fed to D8 on the Arduino
  When this pin is HIGH there is no tone detected.
  When this pin is LOW a tone of the set frequency has been detected.
