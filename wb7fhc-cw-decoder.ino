/***********************************************************************
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

  
*********************************************************************/

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

int audioPin = 8;         // we read data from the tone detector module here
int audio = 1;            // will store the value we read on this pin

int LCDline = 1;          // keeps track of which line we're printing on
int lineEnd = 21;         // One more than number of characters across display
int letterCount = 0;      // keeps track of how may characters were printed on the line
int lastWordCount = 0;    // keeps track of how may characters are in the current word
int lastSpace = 0;        // keeps track of the location of the last 'space'

// The next line stores the text that we are currently printing on a line,
// The charcters in the current word,
// Our top line of text,
// Our second line of text,
// and our third line of text
// For a 20x4 display these are all 20 characters long
char currentLine[] = "12345678901234567890";
char    lastWord[] = "                    ";
char       line1[] = "                    ";
char       line2[] = "                    ";
char       line3[] = "                    ";

boolean ditOrDah = true;  // We have either a full dit or a full dah
int dit = 10;             // We start by defining a dit as 10 milliseconds

// The following values will auto adjust to the sender's speed
int averageDah = 240;             // A dah should be 3 times as long as a dit
int averageWordGap = averageDah;  // will auto adjust
long fullWait = 6000;             // The time between letters
long waitWait = 6000;             // The time between dits and dahs
long newWord = 0;                 // The time between words

boolean characterDone = true; // A full character has been sent

int downTime = 0;        // How long the tone was on in milliseconds
int upTime = 0;          // How long the tone was off in milliseconds
int myBounce = 2;        // Used as a short delay between key up and down

long startDownTime = 0;  // Arduino's internal timer when tone first comes on
long startUpTime = 0;    // Arduino's internal timer when tone first goes off

long lastDahTime = 0;    // Length of last dah in milliseconds
long lastDitTime = 0;    // Length oflast dit in milliseconds
long averageDahTime = 0; // Sloppy Average of length of dahs

boolean justDid = true; // Makes sure we only print one space during long gaps

int myNum = 0;           // We will turn dits and dahs into a binary number stored here

/////////////////////////////////////////////////////////////////////////////////
// Now here is the 'Secret Sauce'
// The Morse Code is embedded into the binary version of the numbers from 2 - 63
// The place a letter appears here matches myNum that we parsed out of the code
// #'s are miscopied characters
char mySet[] ="##TEMNAIOGKDWRUS##QZYCXBJP#L#FVH09#8###7#####/-61#######2###3#45";
char lcdGuy = ' ';       // We will store the actual character decoded here

/////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(audioPin, INPUT);
  pinMode(13,OUTPUT);    // We're going to blink Arduino's onboard LED
  lcd.begin(20, 4);      // Cuzz we have a 20x4 display
  lcd.clear();           // Get rid of any garbage that might appear on startup
  //delay(2000);
  lcd.print("WB7FHC CW DECODER v5");
  lcd.setCursor(0,1);
 
}

 void loop() {
   audio = digitalRead(audioPin); // What is the tone decoder doing?

   if (!audio) keyIsDown();       // LOW, or 0, means tone is being decoded
   if (audio) keyIsUp();          // HIGH, or 1, means no tone is there
 }

 void keyIsDown() {
   // The decoder is detecting our tone
   // The LEDs on the decoder and Arduino will blink on in unison
   digitalWrite(13,1);            // turn on Arduino's LED
   
   
   if (startUpTime>0){
     // We only need to do once, when the key first goes down
     startUpTime=0;    // clear the 'Key Up' timer
     }
   // If we haven't already started our timer, do it now
   if (startDownTime == 0){
       startDownTime = millis();  // get Arduino's current clock time
      }

     characterDone=false; // we're still building a character
     ditOrDah=false;      // the key is still down we're not done with the tone
     delay(myBounce);     // Take a short breath here
     
   if (myNum == 0) {      // myNum will equal zero at the beginning of a character
      myNum = 1;          // This is our start bit  - it only does this once per letter
      }
 }
 
  void keyIsUp() {
   // The decoder does not detect our tone
   // The LEDs on the decoder and Arduino will blink off in unison 
   digitalWrite(13,0);    // turn off Arduino's LED
   
   // If we haven't already started our timer, do it now
   if (startUpTime == 0){startUpTime = millis();}
   
   // Find out how long we've gone with no tone
   // If it is twice as long as a dah print a space
   upTime = millis() - startUpTime;
   if (upTime<10)return;
   if (upTime > (averageDah*2)) {    
      printSpace();
   }
   
   // Only do this once after the key goes up
   if (startDownTime > 0){
     downTime = millis() - startDownTime;  // how long was the tone on?
     startDownTime=0;      // clear the 'Key Down' timer
   }
 
   if (!ditOrDah) {   
     // We don't know if it was a dit or a dah yet
      shiftBits();    // let's go find out! And do our Magic with the bits
    }

    // If we are still building a character ...
    if (!characterDone) {
       // Are we done yet?
       if (upTime > dit) { 
         // BINGO! we're done with this one  
         printCharacter();       // Go figure out what character it was and print it       
         characterDone=true;     // We got him, we're done here
         myNum=0;                // This sets us up for getting the next start bit
         }
         downTime=0;               // Reset our keyDown counter
       }
   }
   
   
void shiftBits() {
  // we know we've got a dit or a dah, let's find out which
  // then we will shift the bits in myNum and then add 1 or not add 1
  
  if (downTime < dit / 3) return;  // ignore my keybounce
  
  myNum = myNum << 1;   // shift bits left
  ditOrDah = true;        // we will know which one in two lines 
  
  
  // If it is a dit we add 1. If it is a dah we do nothing!
  if (downTime < dit) {
     myNum++;           // add one because it is a dit
     } else {
  
    // The next three lines handle the automatic speed adjustment:
    averageDah = (downTime+averageDah) / 2;  // running average of dahs
    dit = averageDah / 3;                    // normal dit would be this
    dit = dit * 2;    // double it to get the threshold between dits and dahs
     }
  }


void printCharacter() {           
  justDid = false;         // OK to print a space again after this
  
  // Punctuation marks will make a BIG myNum
  if (myNum > 63) {  
    printPunctuation();  // The value we parsed is bigger than our character array
                         // It is probably a punctuation mark so go figure it out.
    return;              // Go back to the main loop(), we're done here.
  }
  lcdGuy = mySet[myNum]; // Find the letter in the character set
  sendToLCD();           // Go figure out where to put in on the display
}

void printSpace() {
  if (justDid) return;  // only one space, no matter how long the gap
  justDid = true;       // so we don't do this twice
  
  // We keep track of the average gap between words and bump it up 20 milliseconds
  // do avoid false spaces within the word
  averageWordGap = ((averageWordGap + upTime) / 2) + 20;

  lastWordCount=0;      // start counting length of word again
  currentLine[letterCount]=' ';  // and a space to the variable that stores the current line
  lastSpace=letterCount;         // keep track of this, our last, space
  
  // Now we need to clear all the characters out of our last word array
  for (int i=0; i<20; i++) {
    lastWord[i]=' ';
   }
   
  lcdGuy=' ';            // this is going to go to the LCD 
  
  // We don't need to print the space if we are at the very end of the line
  if (letterCount < 20) { 
    sendToLCD();         // go figure out where to put it on the display
 }
}

void printPunctuation() {
  // Punctuation marks are made up of more dits and dahs than
  // letters and numbers. Rather than extend the character array
  // out to reach these higher numbers we will simply check for
  // them here. This funtion only gets called when myNum is greater than 63
  
  // Thanks to Jack Purdum for the changes in this function
  // The original uses if then statements and only had 3 punctuation
  // marks. Then as I was copying code off of web sites I added
  // characters we don't normally see on the air and the list got
  // a little long. Using 'switch' to handle them is much better.


  switch (myNum) {
    case 71:
      lcdGuy = ':';
      break;
    case 76:
      lcdGuy = ',';
      break;
    case 84:
      lcdGuy = '!';
      break;
    case 94:
      lcdGuy = '-';
      break;
    case 97:
      lcdGuy = 39;    // Apostrophe
      break;
    case 101:
      lcdGuy = '@';
      break;
    case 106:
      lcdGuy = '.';
      break;
    case 115:
      lcdGuy = '?';
      break;
    case 246:
      lcdGuy = '$';
      break;
    case 122:
      lcdGuy = 's';
      sendToLCD();
      lcdGuy = 'k';
      break;
    default:
      lcdGuy = '#';    // Should not get here
      break;
  }
  sendToLCD();    // go figure out where to put it on the display
}

void sendToLCD(){
  // Do this only if the character is a 'space'
  if (lcdGuy > ' '){
   lastWord[lastWordCount] = lcdGuy; // store the space at the end of the array
   if (lastWordCount < lineEnd - 1) {
     lastWordCount++;   // only bump up the counter if we haven't reached the end of the line
   }
  }
  currentLine[letterCount] = lcdGuy; // now store the character in our current line array
 
  letterCount++;                     // we're counting the number of characters on the line

  // If we have reached the end of the line we will go do some chores
  if (letterCount == lineEnd) {
    newLine();  // check for word wrap and get ready for the next line
    return;     // so we don't need to do anything more here
  }
  
  lcd.print(lcdGuy); // print our character at the current cursor location
 
}

//////////////////////////////////////////////////////////////////////////////////////////
// The following functions handle word wrapping and line scrolling for a 4 line display //
//////////////////////////////////////////////////////////////////////////////////////////

void newLine() {
  // sendToLCD() will call this routine when we reach the end of the line
  if (lastSpace == 0){
    // We just printed an entire line without any spaces in it.
    // We cannot word wrap this one so this character has to go at 
    // the beginning of the next line.
    
    // First we need to clear all the characters out of our last word array
    for (int i=0; i<20; i++) {
      lastWord[i]=' ';
     }
     
     lastWord[0]=lcdGuy;  // store this character in the first position of our next word
     lastWordCount=1;     // set the length to 1
   }
  
  truncateOverFlow();    // Trim off the first part of a word that needs to go on the next line
  linePrep();            // Store the current line so we can move it up later
  reprintOverFlow();     // Print the truncated text and space padding on the next line 
  }

void truncateOverFlow(){
  // Our word is running off the end of the line so we will
  // chop it off at the last space and put it at the beginning of the next line
  
  if (lastSpace==0) {return;}  // Don't do this if there was no space in the last line
  
  // Move the cursor to the place where the last space was printed on the current line
  lcd.setCursor(lastSpace,LCDline);
  
  letterCount = lastSpace;    // Change the letter count to this new shorter length
  
  // Print 'spaces' over the top of all the letters we don't want here any more
  for (int i = lastSpace; i < 20; i++) {
     lcd.print(' ');         // This space goes on the display
     currentLine[i] = ' ';   // This space goes in our array
  }
}


void linePrep(){
     LCDline++;           // This is our line number, we make it one higher
     
     // What we do next depends on which line we are moving to
     // The first three cases are pretty simple because we working on a cleared
     // screen. When we get to the bottom, though, we need to do more.
     switch (LCDline) {
     case 1:
       // We just finished line 0
       // don't need to do anything because this for the top line
       // it is going to be thrown out when we scroll anyway.
       break;
     case 2:
       // We just finished line 1
       // We are going to move the contents of our current line into the line1 array
       for (int j=0; j<20; j++){
         line1[j] = currentLine[j];
       }
        break;
     case 3:
       // We just finished line 2
       // We are going to move the contents of our current line into the line2 holding bin
       for (int j=0; j<20; j++){
         line2[j] = currentLine[j];
       }
       break;
     case 4:
       // We just finished line 3
       // We are going to move the contents of our current line into the line3 holding bin
       for (int j=0; j<20; j++){
         line3[j] = currentLine[j];
       }
       //This is our bottom line so we will keep coming back here
       LCDline = 3;  //repeat this line over and over now. There is no such thing as line 4
       
       myScroll();  //move everything up a line so we can do the bottom one again
       break;
   }
        
}

void myScroll(){
  // We will move each line of text up one row
  
  int i = 0;  // we will use this variables in all our for loops
  
  lcd.setCursor(0,0);      // Move the cursor to the top left corner of the display
  lcd.print(line1);        // Print line1 here. Line1 is our second line,
                           // our top line is line0 ... on the next scroll
                           // we toss this away so we don't store line0 anywhere
 
  // Move everything stored in our line2 array into our line1 array
  for (i = 0; i < 20; i++) {
    line1[i] = line2[i];
  }
  
  lcd.setCursor(0,1);      // Move the cursor to the beginning of the second line
  lcd.print(line1);        // Print the new line1 here
 
  // Move everything stored in our line3 array into our line2 array
  for (i = 0; i < 20; i++) {
    line2[i]=line3[i];
  }
  lcd.setCursor(0,2);      // Move the cursor to the beginning of the third line
  lcd.print(line2);        // Print the new line2 here
 
  // Move everything stored in our currentLine array into our line3 array
  for (i = 0; i < 20; i++) {
    line3[i] = currentLine[i];
  }
 
}

void reprintOverFlow(){
  // Here we put the word that wouldn't fit at the end of the previous line
  // Back on the display at the beginning of the new line
  
  // Load up our current line array with what we have so far
   for (int i = 0; i < 20; i++) {
     currentLine[i] = lastWord[i];
  } 
  lcd.setCursor(0, LCDline);              // Move the cursor to the beginning of our new line 
  lcd.print(lastWord);                    // Print the stuff we just took off the previous line
  letterCount = lastWordCount;            // Set up our character counter to match the text
  lcd.setCursor(letterCount, LCDline); 
  lastSpace=0;          // clear the last space pointer
  lastWordCount=0;      // clear the last word length
}




