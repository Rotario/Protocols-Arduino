/*
DESCRIPTION:
 This file is used to measure pulse modulated fluorescence (PMF) using a saturating pulse and a measuring pulse.  The measuring pulse LED (for example, Rebel Luxeon Orange) can itself produce some infra-red
 fluorescence which is detected by the detector.  This signal causes the detector response to be higher than expected which causes bias in the results.  So, in order to account for this added infra-red, we 
 perform a calibration.  The calibration is a short set of pulses which occur at the beginning of the program from an infra-red LED (810nm).  The detector response from these flashes indicates how "reflective"
 the sample is to infra-red light in the range produced by our measuring LED.  We can create a calibration curve by measuring a very reflective sample (like tin foil) and a very non-reflective sample (like
 black electrical tape) with both the calibrating LED and the measuring LED.  This curve tells us when we see detector response X for the calibrating LED, that correspond to a baseline response of Y for the
 measuring LED.  Once we have this calibration curve, when we measure an actual sample (like a plant leaf) we automatically calculate and remove the reflected IR produced by the measuring pulse.  Vioala!
 
 The LED on/off cycles are taken care of by using interval timers.  One timer turns the measuring pulse on (and takes a measurement using analogRead()), one timer turns it off, and one timer controls the length 
 of the whole run.  
 
 for dirkf you need to put the counters in the inside loop - now they are on the outside loop which won't work for loop >1.
 update Basic Fluor
 Add methods for CO2 measurement, light, humidity, etc.
 Add Serial1.print for all available methods at top
 Figure out low power mode
 
 RECENT UPDATES:
 * Changed input structure of file so you choose between 000 - 999 to select what type of measurement you want to run
 * No longer saves any data to SD card (left in the SD card initialization in case we want to add it back in, but sd card is no longer requires)
 * All data is pushed to bluetooth, according to defined structure (<DATA>...</DATA>... etc.)
 * No longer pushes time data from the Teensy itself - time will be pulled from the phone
 
 Using Arduino 1.0.3 w/ Teensyduino installed downloaded from http://www.pjrc.com/teensy/td_download.html .   
 */

// NOTES FOR USER
// When changing protocols, makes sure to change the name of the saved file (ie data-I.csv) so you know what it is.
// Max file name size is 12 TOTAL characters (8 characters + .csv extension).  Program does not distinguish between upper and lower case
// Files have a basename (like ALGAE), and then for each subroutine an extension is added (like ALGAE-I) to indicate from which subroutine the data came.  
// When you create a new experiment, you save the data (ALGAE-I) in a folder.  If you want stop and restart but save data to the same file, you may append that file.
// Otherwise, you can create a new data file (like ALGAE-I) file an a different folder, which will be named 01GAE, 02GAE, 03GAE and so on.
// Calibration is performed to create an accurate reflectance in the sample using a 850nm LED.  You can find a full explanation of the calibration at https://opendesignengine.net/documents/14
// A new calibration should be performed when sample conditions have changed (a new sample is used, the same sample is used in a different position)
// The previous calibration value is saved in the SD card, so if no calibration is performed the most recently saved calibration value will be used!
// See KEY VARIABLES USED IN THE PROTOCOL below to change the measurement pulse cycles, pulse lengths, saturation pulses, etc.
// Note that the output values minimum and maximum are dependent on the analog resolution.  From them, you can calculate the actual current through the detector.
// ... So - at 10 bit resolution, min = 0, max = 10^2 = 1023; 16 bit resolution, min = 0, max = 16^2 = 65535; etc.
// ... To calculate actual voltage on the analog pin: 3.3*((measured value) / (max value based on analog resolution)).
// If you want to dig in and change values in this file, search for "NOTE!".  These areas provide key information that you will need to make changes.
// Pin A10 and A11 are 16 bit enabed with some added coding in C - the other pins cannot achieve 16 bit resolution.
// Real Time clock - includes sync to real time clock in ASCII 10 digit format and printed time with each sample (ie T144334434)

// SPECS USING THIS METHOD: 
// Timing of Measuring pulse and Saturation pulse is within 500ns.  Peak to Peak variability, ON and OFF length variability all is <1us.  Total measurement variability = 0 to +2us (regardless of how many cycles)

// DATASHEETS
// CO2 sensor hardware http://CO2meters.com/Documentation/Datasheets/DS-S8-PSP1081.pdf
// CO2 sensor communication http://CO2meters.com/Documentation/Datasheets/DS-S8-Modbus-rev_P01_1_00.pdf


// SD CARD ENABLE AND SET
#include <Time.h>   // enable real time clock library
#include <Wire.h>
#include <EEPROM.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
//#include <sleep.h>

// SHARED VARIABLES

char filename[13] = "ALGAE"; // Base filename used for data files and directories on the SD Card
char* protocolversion = "001"; // Current long term support version of this file
const char* variable1name = "Fs"; // Fs
const char* variable2name = "Fm"; // Fm (peak value during saturation)
const char* variable3name = "Fd"; // Fd (value when actinic light is off
const char* variable4name = "Fv"; // == Fs-Fo
const char* variable5name = "Phi2"; // == Fv/Fm
const char* variable6name = "1/Fs-1/Fd"; // == 1/Fs-1/Fd
unsigned long starttimer0, starttimer1, starttimer2;
float Fs;
float Fd;
float Fm;
float Phi2;
float invFsinvFd;

// DIRKF VARIABLES

int drepeatrun = 1; // number of times the measurement is repeated
int ddelayruns = 0; // millisecond delay between each repeated run
int dmeasurements = 3; // # of measurements per pulse to be averaged (min 1 measurement per 6us pulselengthon)
int drunlength = 2; // in seconds... minimum = cyclelength
volatile unsigned long dpulselengthon = 30; // pulse length in us... minimum = 6us
volatile float dcyclelength = 10000; // time between cycles of pulse/actinicoff/pulse/actinic on in us... minimum = pulselengthon + 7.5us
volatile int dactinicoff = 500; // in us... length of time actinic is turned off
volatile int dsaturatingcycleon = 50; // The cycle number in which the saturating light turns on (set = 0 for no saturating light) - NOTE! This number is twice the number of the stored value (so this counts 200 cycles to produce a graph with only 100 points, because values are saved in alternating cycles)... so if you expect to have 100 graphed points and you want saturating to start at 25 and end at 50, then set it here to start at 50 and end at 100.
volatile int dsaturatingcycleoff = 100; //The cycle number in which the saturating light turns off
const char* dirkfending = "-D.CSV"; // filename ending for the basicfluor subroutine - just make sure it's no more than 6 digits total including the .CSV
int edge = 2; // The number of values to cut off from the front and back edges of Fm, Fs, Fd.  For example, if Fm starts at 25 and ends at 50, edge = 2 causes it to start at 27 and end at 48.
int* ddatasample1;
int* ddatasample2; 
int* ddatasample3;
int* ddatasample4;

//BASIC FLUORESCENCE VARIABLES
int brepeatrun = 1;
int bmeasurements = 4; // # of measurements per pulse (min 1 measurement per 6us pulselengthon)
unsigned long bpulselengthon = 25; // pulse length in us... minimum = 6us
float bcyclelength = .01; // in seconds... minimum = pulselengthon + 7.5us
float brunlength = 1.5; // in seconds... minimum = cyclelength
const char* basicfluorending = "-B.CSV"; // filename ending for the basicfluor subroutine - just make sure it's no more than 6 digits total including the .CSV
int bsaturatingcycleon = 30; //The cycle number in which the saturating light turns on (set = 0 for no saturating light)
int bsaturatingcycleoff = 100; //The cycle number in which the saturating light turns off
int* bdatasample;

// KEY CALIBRATION VARIABLES
unsigned long calpulsecycles = 50; // Number of times the "pulselengthon" and "pulselengthoff" cycle during calibration (on/off is 1 cycle)
// data for measuring and saturating pulses --> to calculate total time=pulsecycles*(pulselengthon + pulselengthoff)
unsigned long calpulselengthon = 30; // Pulse LED on length for calibration in uS (minimum = 5us based on a single ~4us analogRead - +5us for each additional analogRead measurement in the pulse).
unsigned long calpulselengthoff = 49970; // Pulse LED off length for calibration in uS (minimum = 20us + any additional operations which you may want to call during that time).
unsigned long cmeasurements = 4; // # of measurements per pulse (min 1 measurement per 6us pulselengthon)

// PIN DEFINITIONS AND TEENSY SETTINGS
float reference = 1.2; // The reference (AREF) supplied to the ADC - currently set to INTERNAL = 1.2V
int analogresolution = 16; // Set the resolution of the analog to digital converter (max 16 bit, 13 bit usable)  
/*
// for old unit
 int measuringlight1 = 3; // Teensy pin for measuring light
 int saturatinglight1 = 4; // Teensy pin for saturating light
 int calibratinglight1 = 2; // Teensy pin for calibrating light
 int actiniclight1 = 5; // Teensy pin for actinic light
 */
// for new unit
int measuringlight1 = 15; // Teensy pin for measuring light
int measuringlight2 = 16; // Teensy pin for measuring light
int saturatinglight1 = 20;
int calibratinglight1 = 14;
int actiniclight1; // Teensy pin for actinic light - not currently set

int measuringlight_pwm = 23;
int calibratinglight1_pwm = 9;
int saturatinglight1_intensity2 = 3;
int saturatinglight1_intensity1 = 4;
int saturatinglight1_intensity_switch = 5;
int detector1 = A10; // Teensy analog pin for detector

// HTU21D Temp/Humidity variables
#define temphumid_address 0x40 // HTU21d Temp/hum I2C sensor address
int sck = 19; // clock pin
int sda = 18; // data pin
int wait = 200; // typical delay to let device finish working before requesting the data
unsigned int tempval;
unsigned int rhval;
float temperature;
float rh;

// S8 CO2 variables
byte readCO2[] = {
  0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read CO2 (see app note)
byte response[] = {
  0,0,0,0,0,0,0};  //create an array to store CO2 response
float valMultiplier = 0.1;
int CO2calibration = 17; // manual CO2 calibration pin (CO2 sensor has auto-calibration, so manual calibration is only necessary when you don't want to wait for autocalibration to occur - see datasheet for details 
unsigned long valCO2;

// INTERNAL VARIABLES, COUNTERS, ETC.
volatile unsigned long start1,start1orig,end1, start3, end3, calstart1orig, calend1, start5, start6, start7, end5;
unsigned long pulselengthoncheck, pulselengthoffcheck, pulsecyclescheck, totaltimecheck, caltotaltimecheck;
volatile float data1f, data2f, data3f, data4f, irtinvalue, irtapevalue, rebeltapevalue, rebeltinvalue, irsamplevalue, rebelsamplevalue, baselineir, dataaverage, caldataaverage1, caldataaverage2, rebelslope, irslope, baseline = 0;
char filenamedir[13];
char filenamelocal[13];
volatile int data0=0, data1=0, data2=0, data3=0, data4=0, data5=0, data6=0, data7=0, data8=0, data9=0, data10=0, caldata1, caldata2, caldata3, caldata4, analogresolutionvalue, dpulse1count, dpulse1noactcount, dpulse2count;
int i=0, j=0, k=0,z=0,y=0,q=0,x=0,p=0; // Used as a counters
int* caldatatape; 
int* caldatatin; 
int* caldatasample;
int* rebeldatasample;
int val=0, cal=0, cal2=0, cal3=0, val2=0, flag=0, flag2=0, keypress=0, protocol, protocols, manualadjust=80;
IntervalTimer timer0, timer1, timer2;
char c;

void setup() {
  Serial.begin(115200); // set baud rate for Serial communication to computer via USB
  Serial1.begin(115200); // set baud rate for bluetooth communication on pins 0,1
  Serial3.begin(9600);
  Wire.begin();
  delay(wait);
  delay(3000);

  pinMode(measuringlight1, OUTPUT); // set pin to output
  pinMode(measuringlight2, OUTPUT); // set pin to output
  pinMode(saturatinglight1, OUTPUT); // set pin to output
  pinMode(calibratinglight1, OUTPUT); // set pin to output  
  pinMode(saturatinglight1_intensity2, OUTPUT); // set pin to output
  pinMode(saturatinglight1_intensity1, OUTPUT); // set pin to output
  pinMode(saturatinglight1_intensity_switch, OUTPUT); // set pin to output
  pinMode(calibratinglight1_pwm, OUTPUT); // set pin to output  
  //  pinMode(actiniclight1, OUTPUT); // set pin to output (currently unset)
  analogReadAveraging(1); // set analog averaging to 1 (ie ADC takes only one signal, takes ~3u
  pinMode(detector1, EXTERNAL);
  analogReadRes(analogresolution);
  analogresolutionvalue = pow(2,analogresolution); // calculate the max analogread value of the resolution setting

}

int Protocol() {
  int a;
  int c;
  for (i=0;i<3;i++){
    if (Serial1.available() == 0) {
      c = Serial.read()-48;
    }
    else {
      c = Serial1.read()-48;
    }
    a = (10 * a) + c;
  }
  return a;
}

void loop() {

  //Serial1.println("Please select a 3 digit protocol code to begin a new protocol");
  //Serial1.println("");

  Serial.println("Please select a 3 digit protocol code to begin");
  Serial.println("");

  int num = 3;


  /*
FOOLING AROUND
   
   int num = 3;
   
   Serial1.print("$$$");
   delay(100);
   Serial1.available();
   
   while(1) {}
   
   
   for (x=0;x<num;x++) {
   Serial1.print(Serial1.available());
   }
   */

  while (Serial1.available()<num && Serial.available()<num) {
    //  Serial1.print(Serial1.available());
    //  Serial.print(Serial1.available());
    //  Serial.println(Serial.available());
    //  delay(1000);
    //  idle();
  }

  protocol = Protocol(); // Retreive the 3 digit protocol code 000 - 999
  //protocol = 1; // Retreive the 3 digit protocol code 000 - 999
  //Serial.print(protocol);

  switch(protocol) {
  case 999:        // END TRANSMISSION
    break;

  case 998:        // NULL RETURN
    Serial.println("");
    Serial.println("nothing happens - please use bluetooth for serial communication!");
    Serial1.println("");
    Serial1.println("nothing happens");
    break;

  case 000:        // CALIBRATION
    calibration();
    break;

  case 001:        // TEMP, HUM, CO2
    //  Serial1.println();
    //Begin JSON file printed to bluetooth on Serial1
    Serial1.print("{\"device_version\": 1,");

    Serial.println();
    temp();
    relh();
//    Co2();
    dirkf();
    delay(1000);
    Serial.println();

    //end JSON file printed to bluetooth on Serial1
    Serial1.print("}");
    break;

  case 002:        // DIRK-F
    Co2_evolution();
    break;

  case 003:        // BASIC FLUORESCENCE
    basicfluor();    
    break;

  case 004:        // WASP
    eeprom();
    break;

  case 005:        // eeprom tests
    CO2cal();
    break;
  }

}

void CO2cal() {
  Serial.print("place detector in fresh air (not in house or building) for 30 seconds, then press any button. Make sure sensor environment is steady and calm!");
  Serial1.print("place detector in fresh air (not in house or building) for 30 seconds, then press any button.  Make sure sensor environment is steady and calm!");
  while (Serial1.available()<1 && Serial.available()<1) {
  }
  Serial.print("please wait about 6 seconds");
  Serial1.print("please wait about 6 seconds");
  digitalWriteFast(CO2calibration, HIGH);
  delay(6000);
  digitalWriteFast(CO2calibration, LOW);
  Serial.print("place detector in 0% CO2 air for 30 seconds, then press any button.  Make sure sensor environment is steady and calm!");
  Serial1.print("place detector in 0% CO2 air for 30 seconds, then press any button.  Make sure sensor environment is steady and calm!");
  while (Serial1.available()<2 && Serial.available()<2) {
  }
  Serial.print("please wait about 20 seconds");
  Serial1.print("please wait about 20 seconds");
  digitalWriteFast(CO2calibration, HIGH);
  delay(20000);
  digitalWriteFast(CO2calibration, LOW);
  Serial.print("calibration complete!");
  Serial1.print("calibration complete!");
}

void Co2_evolution() {
  //  measure every second, compare current measurement to previous measurement.  If delta is <x, then stop.
  //  save the dif between measurement 1 and 2 (call CO2_slope), save measurement at final point (call CO2_final), save all points (call CO2_raw)
  // create a raw file based on 300 seconds measurement to stop, then fill in ramaining
  // data with the final value...

  int* co2_raw;
  int co2_maxsize = 150;
  int co2_start; // first CO2 measurement
  int co2_end;
  float co2_drop;
  int co2_slope;
  int co2_2; // second CO2 measurement
  int delta = 20; // measured in ppm
  int delta_min = 1; // minimum difference to end measurement, in ppm
  int co2_time = 1; // measured in seconds
  int c = 0; // counter
  int valCO2_prev;
  int valCO2_prev2;

  co2_raw = (int*)malloc(co2_maxsize*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle

  analogWrite(saturatinglight1_intensity1, 1); // set saturating light intensity
  digitalWriteFast(saturatinglight1_intensity_switch, LOW); // turn intensity 1 on
  digitalWriteFast(saturatinglight1, HIGH);
  
  for (x=0;x<co2_maxsize;x++) {
    requestCo2(readCO2);
    valCO2_prev2 = valCO2_prev;
    valCO2_prev = valCO2;
    valCO2 = getCo2(response);
    Serial.print(valCO2);
    Serial.print(",");
    delta = (valCO2_prev2-valCO2_prev)+(valCO2_prev - valCO2);
    Serial.println(delta);
    c = c+1;
    if (c == 1) {
      co2_start = valCO2;
    }
    else if (c == 2) {
      co2_2 = valCO2;
    }
/*
    else if (delta <= delta_min) {
      co2_end = valCO2;
      co2_drop = (co2_start-co2_end)/co2_start;
      Serial.println("baseline is reached!");
      Serial.print("it took this many seconds: ");
      Serial.println(x);
      Serial.print("CO2 dropped from ");
      Serial.print(co2_start);
      Serial.print(" to ");
      Serial.print(co2_end);
      Serial.print(" a % drop of ");    
      Serial.println(co2_drop, 3);
      break;
    }
*/
    else if (c == co2_maxsize-1) {
      co2_end = valCO2;
      co2_drop = (co2_start-co2_end)/co2_start;
      Serial.print("maximum measurement time exceeded!  Try a larger leaf, a smaller space, or check that the CO2 detector is working properly.");
      Serial.print("it took this many seconds: ");
      Serial.println(x);
      Serial.print("CO2 dropped from ");
      Serial.print(co2_start);
      Serial.print(" to ");
      Serial.print(co2_end);
      Serial.print(" a % drop of ");    
      Serial.println(co2_drop, 3);
    }
    delay(2000);
  }
  Serial.print(x); 
  digitalWriteFast(saturatinglight1, LOW);
  analogWrite(saturatinglight1_intensity1, 0); // set saturating light intensity
}

void Co2() {
  requestCo2(readCO2);
  valCO2 = getCo2(response);
  Serial.print("Co2 ppm = ");
  Serial.println(valCO2);
  Serial1.print("\"co2_content\": ");
  Serial1.print(valCO2);  
  Serial1.print(",");
  delay(100);
}

void requestCo2(byte packet[]) {
  while(!Serial3.available()) { //keep sending request until we start to get a response
    Serial3.write(readCO2,7);
    delay(50);
  }
  int timeout=0;  //set a time out counter
  while(Serial3.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10) {   //if it takes to long there was probably an error
      while(Serial3.available())  //flush whatever we have
          Serial3.read();
      break;                        //exit and try again
    }
    delay(50);
  }
  for (int i=0; i < 7; i++) {
    response[i] = Serial3.read();
  }  
}

unsigned long getCo2(byte packet[]) {
  int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
  int low = packet[4];                         //low byte for value is 5th byte in the packet
  unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
  return val* valMultiplier;
}

int numDigits(int number) {
  int digits = 0;
  if (number < 0) digits = 1; // remove this line if '-' counts as a digit
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}


void relh() {
  Wire.beginTransmission(0x40); // 7 bit address
  Wire.send(0xF5); // trigger temp measurement
  Wire.endTransmission();
  delay(wait);

  // Print response and convert to Celsius:
  Wire.requestFrom(0x40, 2);
  byte byte1 = Wire.read();
  byte byte2 = Wire.read();
  rhval = byte1;
  rhval<<=8; // shift byte 1 to bits 1 - 8
  rhval+=byte2; // put byte 2 into bits 9 - 16
  Serial.print("relative humidity in %: ");
  rh = 125*(rhval/pow(2,16))-6;
  Serial.println(rh);
  Serial1.print("\"relative_humidity\": ");
  Serial1.print(rh);  
  Serial1.print(",");
}

void temp() {
  Wire.beginTransmission(0x40); // 7 bit address
  Wire.send(0xF3); // trigger temp measurement
  Wire.endTransmission();
  delay(wait);

  // Print response and convert to Celsius:
  Wire.requestFrom(0x40, 2);
  byte byte1 = Wire.read();
  byte byte2 = Wire.read();
  tempval = byte1;
  tempval<<=8; // shift byte 1 to bits 1 - 8
  tempval+=byte2; // put byte 2 into bits 9 - 16
  Serial.print("Temperature in Celsius: ");
  temperature = 175.72*(tempval/pow(2,16))-46.85;
  Serial.println(temperature);
  Serial1.print("\"temperature\": ");
  Serial1.print(temperature);  
  Serial1.print(",");
}

void eeprom() {

  int joe = 395;
  char joe2 [10];
  itoa(joe, joe2, 10);

  int digs = numDigits(joe);
  for (i=0;i<digs;i++) {
    //    joe2[i] = (char) joe/pow(10,(i-1));
    //    joe = joe - joe2[i]*pow(10,(i-1));
    Serial1.println(joe2[i]);
  }
  Serial1.println("");
  Serial1.println(joe);
  //  joe2[5] = joe/100000;
  //  joe2[4] = joe/10000;
  // joe2[3] = joe/1000;
  // joe2[2] = joe/100;
  // joe2[1] = joe/10;
  Serial1.println(joe);
  Serial1.println(numDigits(joe));
  delay(50);
  joe = (char) joe;
  Serial1.println(joe);

  int jack;
  EEPROM.write(1,1);
  EEPROM.write(2,2);
  EEPROM.write(3,3);
  EEPROM.write(4,4);  
  jack = EEPROM.read(1);
  Serial1.println(jack);
  jack = EEPROM.read(2);
  Serial1.println(jack);
  jack = EEPROM.read(3);
  Serial1.println(jack);
  jack = EEPROM.read(4);
  Serial1.println(jack);

}

void calibration() {

  analogReadAveraging(cmeasurements); // set analog averaging (ie ADC takes one signal per ~3u)
  Serial1.print("<RUNTIME>");
  Serial1.print("5");
  Serial1.println("</RUNTIME>");


  Serial1.println("Place the shiny side of the calibration piece face up in the photosynq, and close the lid.");
  Serial1.println("When you're done, press any key to continue");
  Serial1.flush();
  while (Serial1.read() <= 0) {
  }

  Serial1.println("Thanks - calibrating...");
  calibrationrebel(); // order matters here - make sure that calibrationrebel() is BEFORE calibrationtin()!
  calibrationtin();

  Serial1.println("");
  Serial1.println("Now place the black side of the calibration piece face up in the photosynq, and close the lid.");
  Serial1.println("");  
  Serial1.println("");

  Serial1.println("When you're done, press any key to continue");
  Serial1.flush();
  while (Serial1.read() <= 0) {
  }

  Serial1.println("Thanks - calibrating...");
  calibrationrebel(); // order matters here - make sure that calibrationrebel() is BEFORE calibrationtape()!
  calibrationtape();
  Serial1.println("Calibration finished!");
  Serial1.println("");
}

void calibrationrebel() {

  // CALIBRATION REBEL
  // Short pulses for calibration using the measuring LED (rebel orange)

  rebeldatasample = (int*)malloc(calpulsecycles*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle
  noInterrupts();

  start1orig = micros();
  start1 = micros();
  for (i=0;i<calpulsecycles;i++) {
    digitalWriteFast(measuringlight1, HIGH); 
    data1 = analogRead(detector1); 
    start1=start1+calpulselengthon;
    while (micros()<start1) {
    }
    start1=start1+calpulselengthoff;
    digitalWriteFast(measuringlight1, LOW); 
    rebeldatasample[i] = data1; 
    while (micros()<start1) {
    } 
  }
  end1 = micros();
  interrupts();

  free(rebeldatasample); // release the memory allocated for the data

  for (i=0;i<calpulsecycles;i++) {
    rebelsamplevalue += rebeldatasample[i]; // totals all of the analogReads taken
  }
  delay(50);

  rebelsamplevalue = (float) rebelsamplevalue; // create a float for rebelsamplevalue so it can be saved later
  rebelsamplevalue = (rebelsamplevalue / calpulsecycles);
  Serial1.print("Rebel sample value:  ");
  Serial1.println(rebelsamplevalue);
  Serial1.println("");  
  for (i=0;i<calpulsecycles;i++) { // Print the results!
    Serial1.print(rebeldatasample[i]);
    Serial1.print(", ");
    Serial1.print(" ");  
  }
  Serial1.println("");
}


void calibrationtin() {

  // CALIBRATION TIN
  // Flash calibrating light to determine how reflective the sample is to ~850nm light with the tin foil as the sample (low reflectance).  This has been tested with Luxeon Rebel Orange as measuring pulse.

  caldatatin = (int*)malloc(calpulsecycles*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle
  noInterrupts(); // turn off interrupts to reduce interference from other calls

    start1 = micros();
  for (i=0;i<calpulsecycles;i++) {
    digitalWriteFast(calibratinglight1, HIGH);
    caldata1 = analogRead(detector1);
    start1=start1+calpulselengthon;
    while (micros()<start1) {
    }
    start1=start1+calpulselengthoff;
    digitalWriteFast(calibratinglight1, LOW); 
    caldatatin[i] = caldata1;  
    while (micros()<start1) {
    } 
  }

  interrupts();

  for (i=0;i<calpulsecycles;i++) {
    irtinvalue += caldatatin[i]; // totals all of the analogReads taken
  }
  Serial1.println(irtinvalue);  
  irtinvalue = (float) irtinvalue;
  irtinvalue = (irtinvalue / calpulsecycles); //  Divide by the total number of samples to get the average reading during the calibration - NOTE! The divisor here needs to be equal to the number of analogReads performed above!!!! 
  rebeltinvalue = rebelsamplevalue;
  rebelsamplevalue = (int) rebelsamplevalue; // reset rebelsamplevalue to integer for future operations
  for (i=0;i<calpulsecycles;i++) { // Print the results!
    Serial1.print(caldatatin[i]);
    Serial1.print(", ");
    Serial1.print(" ");  
  }
  Serial1.println(" ");    
  Serial1.print("the baseline high reflectance value from calibration: ");
  Serial1.println(irtinvalue, 7);
  Serial1.print("The last 4 data points from the calibration: ");  
  Serial1.println(caldata1);
}

void calibrationtape() {

  // CALIBRATION TAPE
  // Flash calibrating light to determine how reflective the sample is to ~850nm light with the black tape as the sample (low reflectance).  This has been tested with Luxeon Rebel Orange as measuring pulse.

  caldatatape = (int*)malloc(calpulsecycles*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle
  noInterrupts(); // turn off interrupts to reduce interference from other calls

    start1 = micros();
  for (i=0;i<calpulsecycles;i++) {
    digitalWriteFast(calibratinglight1, HIGH);
    caldata1 = analogRead(detector1);
    start1=start1+calpulselengthon;
    while (micros()<start1) {
    }
    start1=start1+calpulselengthoff;
    digitalWriteFast(calibratinglight1, LOW);
    caldatatape[i] = caldata1; 
    while (micros()<start1) {
    } 
  }

  interrupts();

  for (i=0;i<calpulsecycles;i++) {
    irtapevalue += caldatatape[i]; // totals all of the analogReads taken
  }
  Serial1.println(irtapevalue);
  irtapevalue = (float) irtapevalue;
  irtapevalue = (irtapevalue / calpulsecycles); 
  rebeltapevalue = rebelsamplevalue;
  rebelsamplevalue = (int) rebelsamplevalue; // reset rebelsamplevalue to integer for future operations 
  for (i=0;i<calpulsecycles;i++) { // Print the results!
    Serial1.print(caldatatape[i]);
    Serial1.print(", ");
    Serial1.print(" ");  
  }
  Serial1.println(" ");    
  Serial1.print("the baseline low reflectance value from calibration:  ");
  Serial1.println(irtapevalue, 7);

  //CALCULATE AND SAVE CALIBRATION DATA TO SD CARD
  rebelslope = rebeltinvalue - rebeltapevalue;
  irslope = irtinvalue - irtapevalue;

  //CALCULATE AND SAVE CALIBRATION DATA TO EEPROM (convert to integer and save decimal places by x10,000)

  Serial1.println("Rebel tape value: ");
  savecalibration(rebeltapevalue, 0);
  Serial1.print("<CAL1>");
  callcalibration(0);
  Serial1.println("</CAL1>");
  Serial1.println("");

  Serial1.println("Rebel tin value: ");
  savecalibration(rebeltinvalue, 10);
  Serial1.print("<CAL2>");
  callcalibration(10);
  Serial1.println("</CAL2>");
  Serial1.println("");

  Serial1.println("IR tape value: ");
  savecalibration(irtapevalue, 20);
  Serial1.print("<CAL3>");
  callcalibration(20);
  Serial1.println("</CAL3>");
  Serial1.println("");

  Serial1.println("IR tin value: ");
  savecalibration(irtinvalue, 30);
  Serial1.print("<CAL4>");
  callcalibration(30);
  Serial1.println("</CAL4>");
  Serial1.println("");

  Serial1.println("Rebel slope value: ");
  savecalibration(rebelslope, 40);
  Serial1.print("<CAL5>");
  callcalibration(40);
  Serial1.println("</CAL5>");
  Serial1.println("");

  Serial1.println("IR slope value: ");
  savecalibration(irslope, 50);
  Serial1.print("<CAL6>");
  callcalibration(50);
  Serial1.println("</CAL6>");
  Serial1.println("");

}


void calibrationsample() {

  // CALIBRATION SAMPLE
  // Flash calibrating light to determine how reflective the sample is to ~850nm light with the actual sample in place.  This has been tested with Luxeon Rebel Orange as measuring pulse.

  caldatasample = (int*)malloc(calpulsecycles*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle
  noInterrupts(); // turn off interrupts to reduce interference from other calls

    calstart1orig = micros();
  start1 = micros();
  for (i=0;i<calpulsecycles;i++) {
    digitalWriteFast(calibratinglight1, HIGH);
    caldata1 = analogRead(detector1);
    start1=start1+calpulselengthon;
    while (micros()<start1) {
    }
    start1=start1+calpulselengthoff;
    digitalWriteFast(calibratinglight1, LOW); 
    caldatasample[i] = caldata1; 
    while (micros()<start1) {
    } 
  }
  calend1 = micros();

  interrupts();

  for (i=0;i<calpulsecycles;i++) {
    irsamplevalue += caldatasample[i]; // totals all of the analogReads taken
  }
  irsamplevalue = (float) irsamplevalue;
  irsamplevalue = (irsamplevalue / calpulsecycles); 
  for (i=0;i<calpulsecycles;i++) { // Print the results!
  }

  // CALCULATE BASELINE VALUE
  baseline = (rebeltapevalue+((irsamplevalue-irtapevalue)/irslope)*rebelslope);

  Serial1.print("\"ir_low\":");
  Serial1.print(irtapevalue);
  Serial1.print(",");
  Serial1.print("\"ir_high\":");
  Serial1.print(irtinvalue);
  Serial1.print(",");
  Serial1.print("\"led_low\":");
  Serial1.print(rebeltapevalue);
  Serial1.print(",");
  Serial1.print("\"led_high\":");
  Serial1.print(rebeltapevalue);
  Serial1.print(",");
  Serial1.print("\"baseline\":");
  Serial1.print(baseline);
  Serial1.print(",");

}

void basicfluor() {
  // Flash the LED in a cycle with defined ON and OFF times, and take analogRead measurements as fast as possible on the ON cycle

  calibrationsample();

  // Set saturating flash intensities via intensity1 and intensity2 (0 - 255)
  analogWrite(saturatinglight1_intensity1, 255); // set saturating light intensity
  analogWrite(saturatinglight1_intensity2, 255); // set saturating light intensity
  digitalWriteFast(saturatinglight1_intensity_switch, LOW); // tu rnintensity 1 on

  analogReadAveraging(bmeasurements); // set analog averaging (ie ADC takes one signal per ~3u)
  Serial1.print("<RUNTIME>");
  Serial1.print(brunlength*brepeatrun);
  Serial1.print("</RUNTIME>");

  for (q=0;q<brepeatrun;q++) {

    Serial1.println("check this out");
    Serial1.println((brunlength/(bcyclelength))*sizeof(int));

    bdatasample = (int*)malloc((brunlength/(bcyclelength))*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle   

    starttimer0 = micros()+100; // This is some arbitrary reasonable value to give the Arduino time before starting
    starttimer1 = starttimer0+bpulselengthon;
    while (micros()<starttimer0) {
    }
    timer0.begin(bpulseon, bcyclelength*1000000); // LED on.  Takes about 1us to call "start" function in PITimer plus 5us per analogRead()
    while (micros()<starttimer1) {
    }
    timer1.begin(bpulseoff, bcyclelength*1000000); // LED off.
    timer2.begin(bstoptimers, brunlength*1000000); // turn off timers after runlength time

    // WAIT FOR TIMERS TO END (give it runlength plus a 10ms to be safe)
    delay(brunlength*1000+10);
    z = 0; // reset counter z

    free(bdatasample); // release the memory allocated for the data

    //delay(30000); // wait 30 seconds for things to mix around
    bcalculations();

    Serial1.print("<END>");
  }
}

void bpulseon() {
  //    Serial1.print(z);
  if (z==(bsaturatingcycleon-1)) { // turn on saturating light at beginning of measuring light
    digitalWriteFast(saturatinglight1, HIGH);
  }
  digitalWriteFast(measuringlight1, HIGH);
  data1 = analogRead(detector1);
  i = 0;
}

void bpulseoff() {

  // NOTE! for very short OFF cycles, just store the data in the datasample[], and write to 
  // the SD card at the end.  If OFF cycle is long enough (50us or more), then you can write
  // directly to the SD card.  The advantage is you are limited to ~1500 data points for
  // datasample[] before it becomes too big for the memory to hold.

  digitalWriteFast(measuringlight1, LOW);
  if (z==(bsaturatingcycleoff-1)) { // turn off saturating light at end of measuring light pulse
    digitalWriteFast(saturatinglight1, LOW);
  }
  bdatasample[z] = data1-baseline; 
  Serial1.print(bdatasample[z]);
  Serial1.print(",");
  //  file.print(bdatasample[z]);
  //  file.print(",");  
  data1 = 0; // reset data1 for the next round
  z=z+1;
}

void bstoptimers() { // Stop timers, close file and directory on SD card, free memory from datasample[], turn off lights, reset counter variables, move to next row in .csv file, 
  timer0.end();
  timer1.end();
  timer2.end();
  end1 = micros();
  digitalWriteFast(measuringlight1, LOW);
  digitalWriteFast(calibratinglight1, LOW);
  digitalWriteFast(saturatinglight1, LOW);
  z=0; // reset counters
  i=0;
  Serial1.println("");
  Serial1.print("Total run time is ~: ");
  Serial1.println(end1-starttimer0);
}

// USER PRINTOUT OF TEST RESULTS
void bcalculations() {

  Serial1.println("DATA - RAW ANALOG VALUES IN uV");
  for (i=0;i<(brunlength/bcyclelength);i++) {
    bdatasample[i] = 1000000*((reference*bdatasample[i])/(analogresolutionvalue*bmeasurements)); 
  }
  for (i=0;i<(brunlength/bcyclelength);i++) { // Print the results!
    Serial1.print(bdatasample[i], 8);
    Serial1.print(",");
  }
  Serial1.println("");

  totaltimecheck = end1 - start1orig;
  caltotaltimecheck = calend1 - calstart1orig;

  Serial1.println("");
  Serial1.print("Size of the baseline:  ");
  Serial1.print("<BASELINE>");
  Serial1.println(baseline, 8);
  Serial1.print("</BASELINE>");

  totaltimecheck = end1 - start1orig;
  caltotaltimecheck = calend1 - calstart1orig;

  Serial1.println("");
  Serial1.println("GENERAL INFORMATION");
  Serial1.println("");

  Serial1.print("total run length (measuring pulses):  ");
  Serial1.println(totaltimecheck);

  Serial1.print("expected run length(measuring pulses):  ");
  Serial1.println(brunlength/bcyclelength);

  Serial1.print("total run length (calibration pulses):  ");
  Serial1.println(caltotaltimecheck);

  Serial1.println("");
  Serial1.println("CALIBRATION DATA");
  Serial1.println("");


  Serial1.print("The baseline from the sample is:  ");
  Serial1.println(baseline);
  Serial1.print("The calibration value using the reflective side for the calibration LED and measuring LED are:  ");
  Serial1.print(rebeltinvalue);
  Serial1.print(" and ");
  Serial1.println(irtinvalue);
  Serial1.print("The calibration value using the black side for the calibration LED and measuring LED are:  ");
  Serial1.print(rebeltapevalue);
  Serial1.print(" and ");
  Serial1.println(irtapevalue);

  delay(50);

}





void dirkf() {
  // Flash the LED in a cycle with defined ON and OFF times, and take analogRead measurements as fast as possible on the ON cycle

  calibrationsample();

  // Set saturating flash intensities via intensity1 and intensity2 (0 - 255)
  //  analogWrite(saturatinglight1_intensity1, 1); // set saturating light intensity
  //  analogWrite(saturatinglight1_intensity2, 255); // set saturating light intensity
  //  digitalWriteFast(saturatinglight1_intensity_switch, LOW); // tu rnintensity 1 on

  analogReadAveraging(dmeasurements); // set analog averaging (ie ADC takes one signal per ~3u)

  ddatasample1 = (int*)malloc((drunlength*1000000/(dcyclelength*2))*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle   
  ddatasample2 = (int*)malloc((drunlength*1000000/(dcyclelength*2))*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle   
  ddatasample3 = (int*)malloc((drunlength*1000000/(dcyclelength*2))*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle   
  ddatasample4 = (int*)malloc((drunlength*1000000/(dcyclelength*2))*sizeof(int)); // create the array of proper size to save one value for all each ON/OFF cycle   

  // TURN ACTINIC LIGHT ON
  digitalWriteFast(actiniclight1, HIGH);  

  for (x=0;x<drepeatrun;x++) {

    // START TIMERS WITH PROPER SEPARATION BETWEEN THEM
    starttimer0 = micros()+100; // This is some arbitrary reasonable value to give the Arduino time before starting
    starttimer1 = starttimer0+dactinicoff;
    while (micros()<starttimer0) {
    }
    timer0.begin(dpulse1,dcyclelength); // First pulse before actinic turns off
    while (micros()<starttimer1) {
    }
    timer1.begin(dpulse2,dcyclelength); // Second pulse, just before actinic turns back on
    delayMicroseconds(dpulselengthon+30); // wait for dpulse2 to end before saving data - Important!  If too short, Teensy will freeze occassionally
    timer2.begin(ddatasave,dcyclelength); // Save data from each pulse

    // WAIT FOR TIMERS TO END (give it runlength plus a 10ms to be safe)
    delay(drunlength*1000+10);

    end1 = micros();

    // STOP AND RESET COUNTERS
    timer0.end();
    timer1.end();
    timer2.end();

    // MAKE SURE ANY REMAINING LIGHTS TURN OFF
    digitalWriteFast(measuringlight1, LOW);
    digitalWriteFast(calibratinglight1, LOW);
    digitalWriteFast(saturatinglight1, LOW);
    digitalWriteFast(actiniclight1, LOW);

    delay(ddelayruns); // wait a little bit
  }

  dcalculations();

  x=0; // Reset counter
  dpulse1count = 0;
  dpulse2count = 0;
  dpulse1noactcount = 0;
  i=0;

  delay(100);
  free(ddatasample1); // release the memory allocated for the data
  delay(100);
  free(ddatasample2); // release the memory allocated for the data
  delay(100);
  free(ddatasample3); // release the memory allocated for the data
  delay(100);
  free(ddatasample4); // release the memory allocated for the data
  delay(100);
}

void dpulse1() {
  start1 = micros();
  digitalWriteFast(measuringlight1, HIGH);
  if (dsaturatingcycleon == dpulse1count) {
    digitalWriteFast(saturatinglight1, HIGH);  // Turn saturating light on
    manualadjust = 210;
  }  
  data0 = analogRead(detector1);
  start1=start1+dpulselengthon;
  while (micros()<start1) {
  }
  digitalWriteFast(measuringlight1, LOW);
  digitalWriteFast(actiniclight1, LOW); // Turn actinic off
  data1 = data0;
  dpulse1count++;
  //  Serial1.println(dpulse1count);
}

void dpulse2() {
  start1 = micros();
  digitalWriteFast(measuringlight1, HIGH);
  data1 = analogRead(detector1);
  start1=start1+dpulselengthon;
  while (micros()<start1) {
  }
  digitalWriteFast(measuringlight1, LOW);
  if (dsaturatingcycleoff == dpulse2count) {
    digitalWriteFast(saturatinglight1, LOW);  // Turn saturating light back on if it was off
    manualadjust = 110;
  }
  digitalWriteFast(actiniclight1, HIGH); // Turn actinic back on
  data2 = data0;
  dpulse2count++;
}

void ddatasave() {

  if (dpulse1count%2 == 1) {
    ddatasample1[(dpulse1count-1)/2] = data1-manualadjust;
    ddatasample2[(dpulse1count-1)/2] = data2-manualadjust;
  }
  if (dpulse1count%2 == 0) {
    ddatasample3[(dpulse1count-1)/2] = data1-manualadjust;
    ddatasample4[(dpulse1count-1)/2] = data2-manualadjust;
  }
}

// USER PRINTOUT OF TEST RESULTS
void dcalculations() {

  for (i=0;i<(dpulse2count/2);i++) { // Print the results!
    Serial.print(ddatasample1[i]);
    Serial.print(",");
  }
  Serial.println(",");

  for (i=0;i<(dpulse2count/2);i++) { // Print the results!
    Serial.print(ddatasample2[i]);
    Serial.print(",");
  }
  Serial.println(",");

  for (i=edge;i<(dsaturatingcycleon/2-edge);i++) { // Print the results!
    Fs += ddatasample3[i];
  }
  Fs = Fs / (dsaturatingcycleon/2-edge);

  for (i=edge;i<(dsaturatingcycleon/2-edge);i++) { // Print the results!
    Fd += ddatasample2[i];
  }
  Fd = Fd / (dsaturatingcycleon/2-edge);

  for (i=dsaturatingcycleon/2+edge;i<(dsaturatingcycleoff/2-edge);i++) { // Print the results!
    Fm += ddatasample3[i];
  }
  Fm = Fm / (dsaturatingcycleoff/2-dsaturatingcycleon/2-2*edge);

  Phi2 = (Fm-Fs)/Fm;

  Serial1.print("\"phi2_raw\": [");

  for (i=0;i<(dpulse2count/2);i++) { // Print the results!
    Serial.print(ddatasample3[i]);
    Serial.print(",");
    Serial1.print(ddatasample3[i]);
    if (i<(dpulse2count/2)-1) {
      Serial1.print(",");
    }
    else {
    }
  }
  Serial.println(",");

  for (i=0;i<(dpulse2count/2);i++) { // Print the results!
    Serial.print(ddatasample4[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial1.print("],");
  Serial1.print(" \"photosynthetic_efficiency_phi2\": ");
  Serial1.print(Phi2,3);
  Serial1.print(",");
  Serial1.print(" \"fs\": ");
  Serial1.print(Fs);
  Serial1.print(",");
  Serial1.print(" \"baseline\": ");
  Serial1.print(baseline);

}

int savecalibration(float calval, int loc) {
  char str [10];
  calval = calval*1000000;
  calval = (int) calval;
  itoa(calval, str, 10);
  for (i=0;i<10;i++) {
    EEPROM.write(loc+i,str[i]);
    //    Serial1.print(str[i]);
    char temp = EEPROM.read(loc+i);
  }
  Serial1.println(str);
}

int callcalibration(int loc) { 
  char temp [10];
  float calval;
  for (i=0;i<10;i++) {
    temp[i] = EEPROM.read(loc+i);
  }
  calval = atoi(temp);
  calval = calval / 1000000;
  Serial1.print(calval,4);
  return calval;
}




