
/*  light-sensor
 *  by Becky Stewart of Anti-Alias Labs
 *  becky@antialiaslabs.com
 *
 *  Initial code by Melissa Coleman
 *  Data logger code based on 
 *  http://www.ladyada.net/make/logshield/lighttempwalkthru.html
 *  We're using the Taos TSL237S-LF light-to-frequency sensor:
 *  http://www.taosinc.com/getfile.aspx?type=press&file=tsl237-e37.pdf
 */
 
 
#include "SD.h"
#include <Wire.h>
#include "RTClib.h"

//These define the pin connections of the Arduino.  
//They can be changed but only use digital in 2 or 3 for the Freq pin
#define TSL_FREQ_PIN 2 // output use digital pin2 for interrupt

float fD = 0.1; // Dark frequency
float Re = 2.3; // Irradiance responsivity
float eff475 = 0.113; // Luminous efficiency at 475nm, for dinoflagellate bioluminescence

RTC_DS1307 RTC; // define the Real Time Clock object

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// the digital pins that connect to the LEDs
#define redLEDpin 3
#define greenLEDpin 4

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  
  while(1);
}

int timing = 1000; // in milliseconds
volatile unsigned long pulse_cnt = 0;
//unsigned long pulse_cnt = 0;

void setup() {
  
  Serial.begin(9600);
  Serial.println();
  Wire.begin();
  RTC.begin();

#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

// initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);
  
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
  logfile.println("millis1,time1,count,uWattpercm2,millis2,time2");    
#if ECHO_TO_SERIAL
  Serial.println("millis1,time1,count,uWattpercm2,millis2,time2");
#endif // attempt to write out the header to the file
  
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  pinMode(TSL_FREQ_PIN, INPUT);
}

void loop() {
  

  float Ee;

  // attach interrupt to pin2, send output pin of TSL230R to arduino 2
  // call handler on each rising pulse

  // High light intensity will trigger interrupts faster than the Arduino can handle, 
  // which may screw up timing of millis() and delay(), which are also interrupt based.
  // To flag excessive interrupts, we keep track of both millis() and the RTC time 
  // before and after our sampling interval.
  
  pulse_cnt=0;
  uint32_t millis1 = millis();
  DateTime time1 = RTC.now();
  
  attachInterrupt(0, add_pulse, RISING);
  delay(timing);
  detachInterrupt(0);

  uint32_t millis2 = millis();
  DateTime time2 = RTC.now();
  unsigned long finalcnt=pulse_cnt;
  
  // Flash green LED at end of sampling interval.
  digitalWrite(greenLEDpin, HIGH);
  delay(500);
  digitalWrite(greenLEDpin, LOW);

  Ee = (finalcnt/(timing/1000.0) - fD)/1000.0/Re; // fO = fD + (Re)(Ee)

  logfile.print(millis1);
  logfile.print(", ");    

  logfile.print(time1.year(), DEC);
  logfile.print("/");
  logfile.print(time1.month(), DEC);
  logfile.print("/");
  logfile.print(time1.day(), DEC);
  logfile.print(" ");
  logfile.print(time1.hour(), DEC);
  logfile.print(":");
  logfile.print(time1.minute(), DEC);
  logfile.print(":");
  logfile.print(time1.second(), DEC);
  logfile.print(", ");

  logfile.print(finalcnt, 10);
  logfile.print(", ");
  
  logfile.print(Ee, 10);
  logfile.print(", ");
  
  logfile.print(millis2);
  logfile.print(", ");    

  logfile.print(time2.year(), DEC);
  logfile.print("/");
  logfile.print(time2.month(), DEC);
  logfile.print("/");
  logfile.print(time2.day(), DEC);
  logfile.print(" ");
  logfile.print(time2.hour(), DEC);
  logfile.print(":");
  logfile.print(time2.minute(), DEC);
  logfile.print(":");
  logfile.print(time2.second(), DEC);
  logfile.println();
  
#if ECHO_TO_SERIAL
  Serial.print(millis1);
  Serial.print(", ");    

  Serial.print(time1.year(), DEC);
  Serial.print("/");
  Serial.print(time1.month(), DEC);
  Serial.print("/");
  Serial.print(time1.day(), DEC);
  Serial.print(" ");
  Serial.print(time1.hour(), DEC);
  Serial.print(":");
  Serial.print(time1.minute(), DEC);
  Serial.print(":");
  Serial.print(time1.second(), DEC);
  Serial.print(", ");

  Serial.print(finalcnt, 10);
  Serial.print(", ");
  
  Serial.print(Ee, 10);
  Serial.print(", ");
  
  Serial.print(millis2);
  Serial.print(", ");    

  Serial.print(time2.year(), DEC);
  Serial.print("/");
  Serial.print(time2.month(), DEC);
  Serial.print("/");
  Serial.print(time2.day(), DEC);
  Serial.print(" ");
  Serial.print(time2.hour(), DEC);
  Serial.print(":");
  Serial.print(time2.minute(), DEC);
  Serial.print(":");
  Serial.print(time2.second(), DEC);
  Serial.println();
#endif
      
  delay(0);
}

void add_pulse() {

  // increase pulse count
  pulse_cnt++;
  //Serial.println(pulse_cnt);
  return;
}
