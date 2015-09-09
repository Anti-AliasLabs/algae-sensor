/* grapher_csv
 *
 *  by Becky Stewart of Anti-Alias Labs
 *  becky@antialiaslabs.com
 *
 *  Reads in stream of numbers from serial
 *  (such as from an Arduino) and saves the
 *  numbers in a csv file with each number on
 *  a line without any commas.
 */

import processing.serial.*;

Serial myPort;  // Create object from Serial class
String dataString = ""; // String of recorded data
boolean recording = false; // flag for if recording data
int xPos = 1;         // horizontal position of the graph


void setup() {
  size( 900, 400 ); // the width and height of the window

  println( Serial.list() );
  String portName = Serial.list()[3];
  print( "Port chosen is " );
  println( portName );
  myPort = new Serial(this, portName, 9600);
  
  background(0);
}

void draw() {

}

void serialEvent(Serial p) {
  // read a byte from the serial port:
  String inString = p.readStringUntil( '\n' );
  if ( inString != null) {
    inString = inString.trim();
    println( inString );
    // draw the graph
    float inFloat = float( inString );
    drawLine( inFloat );
   
   // if also recording 
    if ( recording ) {
      dataString = dataString + inString + ' ';
    }
  }
}

void keyPressed() {
  if ( key == ' ') {
    // if not recording data
    if ( recording==false ) {
      recording = true;
    }
    // if recording data
    else {
      if ( recording ) {
        // generate filename from date and time to not overwrite old data
        String y = Integer.toString( year() );
        String m = Integer.toString( month() );
        String d = Integer.toString( day() );
        String h = Integer.toString( hour() );
        String mt = Integer.toString( minute() );
        String s = Integer.toString( second() );
        String filename = y +'-'+ m+'-'+d+'-'+h+'-'+mt+'-'+s+'-'+".csv";

        // split up the long string into a list 
        String[] list = split(dataString, ' ');
        // write the strings to a file, each on a separate line
        saveStrings(filename, list);

        // switch recording flag
        recording = false;

        // clear data 
        dataString = "";
      }
    }
  }
}


void drawLine(float latestValue) {
  int maxValue = 50000; // change this to readjust height of line
  latestValue = map(latestValue, 0, maxValue, 0, height);
  // draw the line:
  if( recording ) {
    stroke(127, 34, 255);
  } else{ 
   stroke( 150 ); 
  }
  line(xPos, height, xPos, height - latestValue);

  // at the edge of the screen, go back to the beginning:
  if (xPos >= width) {
    xPos = 0;
    background(0);
  } 
  else {
    // increment the horizontal position:
    xPos++;
  }
}

