// FIRMWARE VERSION OF THIS FILE (SAVED TO EEPROM ON FIRMWARE FLASH)
#define DEVICE_FIRMWARE .47
#define DEVICE_NAME "MultispeQ"

/////////////////////CHANGE LOG/////////////////////
/*

Need to fix


on 1017 - redo the reset, make it work.
consolidate the calls together... 
separate out items into separate pages (sensors, etc. etc.)

open issue relating to what serial port to use regarding upgrade_firmware function 1078

- line 3218 in print_cal_userdef I'm using array_length, which must be passed to the function.  Ideally I should be figuring out the array length dynamically, then dividing by the variable type.  This would allow
a much more robust system for saving the values.
- check all averages for environmental - confirm that they are actually averaging things!

So problem is I'm not able to catch the -1+ when loading the buffer with the json (from { to } and { to } ... and so on)... this is a 'dead zone' where user can cancel but cancel gets lost.
I'm also losing the cancel in the delay - if I hit -1+-1+ in delay it won't cancel - somehow that's getting lost as well.
let's deal with the loss of cancel during the delay


 Most recent updates (46):
 - added quit_check during the beginning of the 'average' loop and during each measurement pulse (while waiting for the measurementdistance delay period). 
 - included CRC check, changed the line breaks (got rid of all line breaks except those for the Serial_Print_CRC function itself, which has a single line break)
 - cleaned up all 10xx codes to include CRC check
 - added new print functions Serial_Print, Serial_Print_Line, Serial_Print_Float, Serial_Print_Float_Line which consolidates the USB and bluetooth printing, and adds up the CRC check.
 - included 'over the air' flash code.

 Most recent updates (45):
 - To use the coralspeq full spectromoeter device:
 - set pulses = 1, spec = 1, and meas_lights = 13 in a given pulse set.  This should output 256 values.  At this point, do not set pulses > 1, just create new pulse sets to run multiple spec traces together.
 - you can set actinic and measuring lights as per normal MultispeQ rules.  If you want the lights to warm up first or have delays, just use additional pulse sets.  
 - For delays which do not output a signal, you can set meas_lights == 0 and no output will be produced from that pulse set (nice for very long delays).

 Most recent updates (45):
 - Fixed the lack of averaging in the r,g,b and light_intensity (raw and PAR), and the incorrect use of the averaged par for use in each individually repeated measurement using ambient light.
 - Added in support for coral spec (in "environmental" access with "spec") - replaced normal "data_raw" with the spec data if "spec" is called in the protocol JSON.

 Most recent updates (44):
 - Fixed an issue which caused PAR to be 2x larger than it should be when measuring both PAR and raw light intensity at the same time.

 Most recent updates (43):
 - added more saved values userdef11 - userdef50, each contains 2 floats as normal.  Also expanded the number of printed decimal places of all userdefs to 6 pdecimal places
 - changed i2c speed setting from 1200 to 400 (Wire.begin(I2C_MASTER,0x00,I2C_PINS_18_19,I2C_PULLUP_EXT,I2C_RATE_400) to allow the new compass to work.  This may not be necessary once the compass is integrated into the circuit board.
 - added development versions for measuring the accelerometer and compass as part of the "environmental" json (for example "environmental":[["accel",0],["compass",0]])

 Most recent updates (42):
 - added averages_delay_ms, measurements_delay_ms, protocols_delay_ms
 - returned the data_raw_average (changed in v0.41) back to a unsigned long from unsigned short because it needs to be summed and ends up >64k. 
 
 Most recent updates (41):
 - reduced variability between measurements by changing the delay between timer 1 and timer 2 from a while loop to a delayMicroseconds
 - doubled maximum trace size (from 10k to 20k points) by converting saved output data from long to unsigned short

 Most recent updates (40):
 - fixed auto-power off output to work with chrome app
 
 /////////////////////LICENSE/////////////////////
 GPLv3 license
 by Greg Austic
 If you use this code, please be nice and attribute if possible and when reasonable!
 Libraries from Adafruit were used for TMP006 and TCS34725, as were code snippets from their examples for taking the measurements.  Lots of help from PJRC forum as well.  Thanks Adafruit and Paul and PJRC community!
 
 /////////////////////DESCRIPTION//////////////////////
 Using Arduino v1.0.5 w/ Teensyduino installed downloaded from http://www.pjrc.com/teensy/td_download.html, intended for use with Teensy 3.1
 
 For more details on hardware, apps, and other related software, go to https://github.com/Photosynq .  
 
 This file is used for the MultispeQ handheld spectrophotometers which are used in the PhotosynQ platform.  It is capable of taking a wide variety of measurements, including 
 fluorescence, 810 and 940 dirk, electrochromic shift, SPAD (chlorophyll content), and environmental variables like CO2, ambient temperature, contactless object temperature,
 relative humidity, light intensity, and color temperature.  It then outputs the data in a JSON format through the USB (serial) and Bluetooth (serial1).  It is designed to work
 with the PhotosynQ app and website for data storage and analysis.
 
 //////////////////////  TERMINOLOGY: //////////////////////  
 A 'pulse' is a single pulse of an LED, usually a measuring LED.  Pulses last for 5 - 100us
 A 'cycle' is a set of X number of pulses.  During each cycle, the user can set an actinic or other light on (not pulsing, just on throughout the cycle)
 'averages' are the number of times you repeat the same measurement BUT the values are averaged and only a single value is outputted from the device
 'measuring lights' are LEDs which have the ability to very quickly and cleanly turn on and off.
 'actinic lights' are LEDs which turn on and off slightly less quickly (but still very quickly!) but usually have better heat sinks and more capacity to be on without burning out
 and with less burn in artifacts (change in LED intensity due to the LED heating up)
 'calibrating lights' are IR LEDs which are used to calculate the baseline to improve the quality of measurements by calculating the amount of interfering IR there is from the measuring
 light
 
 //////////////////////  FEATURES AND NOTES: //////////////////////  
 USB power alone is not sufficient!  You must also have an additional power supply which can quickly supply power, like a 9V lithium ion battery.  Power supply may be from 5V - 24V
 Calibration is performed to create an accurate reflectance in the sample using a 940nm LED.  You can find a full explanation of the calibration at https://opendesignengine.net/documents/14
 A new calibration should be performed when sample conditions have changed (a new sample is used, the same sample is used in a different position)
 Pin A10 and A11 are 16 bit enabed with some added coding in C - the other pins cannot achieve 16 bit resolution.
 
 The are mutliple calibrations for the unit:
 1) offset calibration - the detectors have a fixed offset due to the hardware, which must be subtracted from any raw outputs.  This is the only calibration which is actively performed on the device - 
 in other works the output has already had the offset subtracted from the signal.  All other calibrations are simply passed along to the app to be applied in post-processing
 2) baseline calibration - 
 
 ////////////////////// HARDWARE NOTES //////////////////////////
 
 The detector operates with an AC filter, so only pulsing light (<100us pulses) passes through the filter.  Permanent light sources (like the sun or any other constant light) is completely
 filtered out.  So in order to measure absorbance or fluorescence a pulsing light must be used to be detectedb by the detector.  Below are notes on the speed of the measuring lights
 and actinic lights used in the MultispeQ, and the noise level of the detector:
 
 Optical:
 RISE TIME, Measuring: 0.4us
 FALL TIME, Measuring: 0.2us
 RISE TIME Actinic (bright): 2us
 FALL TIME Actinic (bright): 2us
 RISE TIME Actinic (dim): 1.5us
 FALL TIME Actinic (dim): 1.5us
 
 Electrical:
 RISE TIME, Measuring: .4us
 FALL TIME, Measuring: .2us
 RISE TIME Actinic: 1.5us
 FALL TIME Actinic: 2us
 NOISE LEVEL, detector 1: ~300ppm or ~25 detector units from peak to peak
 OVERALL: Excellent results - definitely good enough for the spectroscopic measurements we plant to make (absorbance (ECS), transmittance, fluorescence (PMF), etc.)
 
 //////////////////////  DATASHEETS ////////////////////// 
 CO2 sensor hardware http://CO2meters.com/Documentation/Datasheets/DS-S8-PSP1081.pdf
 CO2 sensor communication http://CO2meters.com/Documentation/Datasheets/DS-S8-Modbus-rev_P01_1_00.pdf
 TCS34725 light color and intensity sensor http://www.adafruit.com/datasheets/TCS34725.pdf
 HTU21D temp/rel humidity sensor http://www.meas-spec.com/downloads/HTU21D.pdf
 
 //////////////////////  I2C Addresses ////////////////////// 
 TMP006 0x42
 TCS34725 0x29
 HTU21D 0x40
 
 */

//#define DEB0UG 1  // uncomment to add full debug features
//#define DEBUGSIMPLE 1  // uncomment to add partial debug features
//#define DAC 1 // uncomment for boards which do not use DAC for light intensity control

#include <Time.h>                                                             // enable real time clock library
//#include <Wire.h>
#include <EEPROM.h>
#include <DS1307RTC.h>                                                        // a basic DS1307 library that returns time as a time_t
#include <Adafruit_Sensor.h>
#include <Adafruit_TMP006.h>
#include <Adafruit_TCS34725.h>
#include <HTU21D.h>
#include <JsonParser.h>
#include <TCS3471.h>
#include <mcp4728.h>
#include <i2c_t3.h>
#include <SoftwareSerial.h>
#include <EEPROMAnything.h>
//#include <typeinfo>
#include <ADC.h>
#include <avr/sleep.h>
#include <LowPower_Teensy3.h>
#include "serial.h"
#include "flasher.h"
#include "crc32.h"


#define ledPin 13

#if defined(__MK20DX128__)
#define FLASH_SIZE              0x20000
#define FLASH_SECTOR_SIZE       0x400
#define FLASH_ID                "fw_teensy3.0"
#elif defined(__MK20DX256__)
#define FLASH_SIZE              0x40000
#define FLASH_SECTOR_SIZE       0x800
#define FLASH_ID                "fw_teensy3.1"
#else
#error NOT SUPPORTED
#endif
// apparently better - thanks to Frank Boesing
#define RAMFUNC  __attribute__ ((section(".fastrun"), noinline, noclone, optimize("Os") ))

#define RESERVE_FLASH (2 * FLASH_SECTOR_SIZE)

//#include <stdio.h>
//#include "flasher.h"

#define S_PORT Serial


void upgrade_firmware(void);
void boot_check(void);

static int flash_hex_line(const char *line);
int parse_hex_line(const char *theline, uint8_t bytes[], unsigned int *addr, unsigned int *num, unsigned int *code);
static int flash_block(uint32_t address, uint32_t *bytes, int count);

//////////////////////ADC INFORMATION AND LOW POWER MODE////////////////////////
ADC *adc = new ADC();
TEENSY3_LP LP = TEENSY3_LP();

//////////////////////DEVICE ID FIRMWARE VERSION////////////////////////
float device_id = 0;
float device_version = 0;
float manufacture_date = 0;

//////////////////////PIN DEFINITIONS AND TEENSY SETTINGS////////////////////////
#define ANALOGRESOLUTION 16
#define MEASURINGLIGHT1 15      // comes installed with 505 cyan z led with a 20 ohm resistor on it.  The working range is 0 - 255 (0 is high, 255 is low)
#define MEASURINGLIGHT2 16      // comes installed with 520 green z led with a 20 ohm resistor on it.  The working range is 0 - 255 (0 is high, 255 is low)
#define MEASURINGLIGHT3 11      // comes installed with 605 amber z led with a 20 ohm resistor on it.  The working range is high 0 to low 80 (around 80 - 90), with zero off safely at 120
#define MEASURINGLIGHT4 12      // comes installed with 3.3k ohm resistor with 940 LED from osram (SF 4045).  The working range is high 0 to low 95 (80 - 100) with zer off safely at 110
#define ACTINICLIGHT1 20
#define ACTINICLIGHT2 2
#define CALIBRATINGLIGHT1 14
#define CALIBRATINGLIGHT2 10
#define ACTINICLIGHT_INTENSITY_SWITCH 5
#define DETECTOR1 34
#define DETECTOR2 35
#define SAMPLE_AND_HOLD 6
#define PWR_OFF_LIGHTS 22           // teensy shutdown pin auto power off
#define PWR_OFF 21           // teensy shutdown pin auto power off for lights power supply (TL1963)
#define BATT_LEVEL 17               // measure battery level
#define DAC_ON 23

//////////////////////PIN DEFINITIONS FOR CORALSPEQ////////////////////////
#define SPEC_GAIN      28
//#define SPEC_EOS       NA
#define SPEC_ST        26
#define SPEC_CLK       25
#define SPEC_VIDEO     A10
//#define LED530         15
//#define LED2200k       16
//#define LED470         20
//#define LED2200K       2

#define SPEC_CHANNELS    256
uint16_t spec_data[SPEC_CHANNELS];
unsigned long spec_data_average[SPEC_CHANNELS];            // saves the averages of each spec measurement
int delay_time = 1;                                         // delay per half clock (in microseconds).  This ultimately conrols the integration time.
int idx = 0;
int read_time = 35;                                        // Amount of time that the analogRead() procedure takes (in microseconds)
int intTime = 100; 
int accumulateMode = false;
int spec_on = 0;                                            // flag to indicate that spec is being used during this measurement

int _meas_light;															 // measuring light to be used during the interrupt
int serial_buffer_size = 5000;                                        // max size of the incoming jsons
int max_jsons = 15;                                                   // max number of protocols per measurement
float all_pins [26] = {
  15,16,11,12,2,20,14,10,34,35,36,37,38,3,4,9,24,25,26,27,28,29,30,31,32,33};
float calibration_slope [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float calibration_yint [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float calibration_slope_factory [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float calibration_yint_factory [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float calibration_baseline_slope [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float calibration_baseline_yint [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float calibration_blank1 [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float calibration_blank2 [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float calibration_other1 [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float calibration_other2 [26] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float userdef0[2] = {
};
float userdef1[2] = {
};
float userdef2[2] = {
};
float userdef3[2] = {
};
float userdef4[2] = {
};
float userdef5[2] = {
};
float userdef6[2] = {
};
float userdef7[2] = {
};
float userdef8[2] = {
};
float userdef9[2] = {
};
float userdef10[2] = {
};
float userdef11[2] = {
};
float userdef12[2] = {
};
float userdef13[2] = {
};
float userdef14[2] = {
};
float userdef15[2] = {
};
float userdef16[2] = {
};
float userdef17[2] = {
};
float userdef18[2] = {
};
float userdef19[2] = {
};
float userdef20[2] = {
};
float userdef21[2] = {
};
float userdef22[2] = {
};
float userdef23[2] = {
};
float userdef24[2] = {
};
float userdef25[2] = {
};
float userdef26[2] = {
};
float userdef27[2] = {
};
float userdef28[2] = {
};
float userdef29[2] = {
};
float userdef30[2] = {
};
float userdef31[2] = {
};
float userdef32[2] = {
};
float userdef33[2] = {
};
float userdef34[2] = {
};
float userdef35[2] = {
};
float userdef36[2] = {
};
float userdef37[2] = {
};
float userdef38[2] = {
};
float userdef39[2] = {
};
float userdef40[2] = {
};
float userdef41[2] = {
};
float userdef42[2] = {
};
float userdef43[2] = {
};
float userdef44[2] = {
};
float userdef45[2] = {
};
float userdef46[3] = {
};
float userdef47[3] = {
};
float userdef48[3] = {
};
float userdef49[3] = {
};
float userdef50[3] = {
};
float pwr_off_ms[2] = {120000,0};                                                // number of milliseconds before unit auto powers down.
//float pwr_off_ms = 120000;                                                // number of milliseconds before unit auto powers down.
int averages = 1;
int pwr_off_state = 0;
int pwr_off_lights_state = 0;
int act_intensity = 0;
int meas_intensity = 0;
int cal_intensity = 0;
JsonArray act_intensities;                         // write to input register of a DAC. channel 0 for low (actinic).  1 step = +3.69uE (271 == 1000uE, 135 == 500uE, 27 == 100uE)
JsonArray meas_intensities;                        // write to input register of a DAC. channel 3 measuring light.  0 (high) - 4095 (low).  2092 = 0.  From 2092 to zero, 1 step = +.2611uE
JsonArray cal_intensities;                        // write to input register of a DAC. channel 2 calibrating light.  0 (low) - 4095 (high).

//////////////////////Shared Variables///////////////////////////
volatile int off = 0, on = 0;
int analogresolutionvalue;
IntervalTimer timer0, timer1, timer2;
unsigned volatile short data1 = 0;
int act_background_light = 13;
float light_y_intercept = 0;
float light_slope = 3;
float offset_34 = 0;                                                          // create detector offset variables
float offset_35 = 0;
float slope_34 = 0;
float yintercept_34 = 0;
float slope_35 = 0;
float yintercept_35 = 0;
char* bt_response = "OKOKlinvorV1.8OKsetPINOKsetnameOK115200"; // Expected response from bt module after programming is done.
float freqtimer0;
float freqtimer1;
float freqtimer2;

//////////////////////HTU21D Temp/Humidity variables///////////////////////////
#define temphumid_address 0x40                                           // HTU21d Temp/hum I2C sensor address
HTU21D htu;                                                              // create class object
unsigned int tempval;
unsigned int rhval;

//////////////////////S8 CO2 Variables///////////////////////////
byte readCO2[] = {
  0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};                             //Command packet to read CO2 (see app note)
byte response[] = {
  0,0,0,0,0,0,0};                                                        //create an array to store CO2 response
float valMultiplier = 1;
int CO2calibration = 17;                                                 // manual CO2 calibration pin (CO2 sensor has auto-calibration, so manual calibration is only necessary when you don't want to wait for autocalibration to occur - see datasheet for details 

////////////////////TMP006 variables - address is 1000010 (adr0 on SDA, adr1 on GND)//////////////////////
Adafruit_TMP006 tmp006(0x42);  // start with a diferent i2c address!  ADR1 is GND, ADR0 is SDA so address is set to 42
float tmp006_cal_S = 6.4;

////////////////////ENVIRONMENTAL variables averages (must be global) //////////////////////
float analog_read_average = 0;
float digital_read_average = 0;
float relative_humidity_average = 0;
float temperature_average = 0;
float objt_average = 0;
float co2_value_average = 0;
float lux_average = 0;
float r_average = 0;
float g_average = 0;
float b_average = 0;
float lux_average_forpar = 0;
float r_average_forpar = 0;
float g_average_forpar = 0;
float b_average_forpar = 0;
float lux_local = 0;
float r_local = 0;
float g_local = 0;
float b_local = 0;

float tryit [5];

// PUSH THIS AS LOCAL VARIABLE WHEN NEW DETECTOR IS FULLY IMPLEMENTED
int detector;                                                                 // detector used in the current pulse

//////////////////////TCS34725 variables/////////////////////////////////
void i2cWrite(byte address, byte count, byte* buffer);
void i2cRead(byte address, byte count, byte* buffer);
TCS3471 TCS3471(i2cWrite, i2cRead);

//////////////////////DAC VARIABLES////////////////////////
mcp4728 dac = mcp4728(0); // instantiate mcp4728 object, Device ID = 0

void setup() {
  Serial.begin(115200);                                                         // set baud rate for Serial, used for communication to computer via USB
#ifdef DEBUGSIMPLE
  Serial_Print_Line("serial works");
#endif
  Serial1.begin(115200);                                                        // set baud rate for Serial 1 used for bluetooth communication on pins 0,1
#ifdef DEBUGSIMPLE
  Serial_Print_Line("serial1 works");
#endif
  Serial3.begin(9600);                                                          // set baud rate for Serial 3 is used to communicate with the CO2 sensor
#ifdef DEBUGSIMPLE
  Serial_Print_Line("serial3 works");
#endif
  delay(500);

  Wire.begin(I2C_MASTER,0x00,I2C_PINS_18_19,I2C_PULLUP_EXT,I2C_RATE_400);      // using alternative wire library
#ifdef DEBUGSIMPLE
  Serial_Print_Line("successfully opened I2C");
#endif
  dac.vdd(5000); // set VDD(mV) of MCP4728 for correct conversion between LSB and Vout

  TCS3471.setWaitTime(200.0);
  TCS3471.setIntegrationTime(700.0);
  TCS3471.setGain(TCS3471_GAIN_1X);
  TCS3471.enable();

  pinMode(29,OUTPUT);

  pinMode(3,INPUT);
  pinMode(MEASURINGLIGHT1, OUTPUT);                                             // set appropriate pins to output
  pinMode(MEASURINGLIGHT2, OUTPUT);
  pinMode(MEASURINGLIGHT3, OUTPUT);
  pinMode(MEASURINGLIGHT4, OUTPUT);
  pinMode(ACTINICLIGHT1, OUTPUT);
  pinMode(ACTINICLIGHT2, OUTPUT);
  pinMode(CALIBRATINGLIGHT1, OUTPUT);
  pinMode(CALIBRATINGLIGHT2, OUTPUT);
  pinMode(ACTINICLIGHT_INTENSITY_SWITCH, OUTPUT);
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, HIGH);                     // preset the switch to the actinic (high) preset position, DAC channel 0
  pinMode(SAMPLE_AND_HOLD, OUTPUT);
  pinMode(PWR_OFF, OUTPUT);
  pinMode(PWR_OFF_LIGHTS, OUTPUT);
  pinMode(BATT_LEVEL, INPUT); 
  digitalWriteFast(PWR_OFF, LOW);                                               // pull high to power off, pull low to keep power.
  digitalWriteFast(PWR_OFF_LIGHTS, HIGH);                                               // pull high to power on, pull low to power down.
  pinMode(13, OUTPUT);
//&&
//  analogReadAveraging(1);                                                       // set analog averaging to 1 (ie ADC takes only one signal, takes ~3u) - this gets changed later by each protocol
  pinMode(DETECTOR1, EXTERNAL);                                                 // use external reference for the detectors
  pinMode(DETECTOR2, EXTERNAL);
//&&
//  analogReadRes(ANALOGRESOLUTION);                                              // set at top of file, should be 16 bit
//  analogresolutionvalue = pow(2,ANALOGRESOLUTION);                              // calculate the max analogread value of the resolution setting
  float default_resolution = 488.28;
  int timer0 [8] = {
    5, 6, 9, 10, 20, 21, 22, 23  };
  int timer1 [2] = {
    3, 4  };
  int timer2 [2] = {
    25, 32  }; 
  analogWriteFrequency(3, 187500);                                              // Pins 3 and 5 are each on timer 0 and 1, respectively.  This will automatically convert all other pwm pins to the same frequency.
  analogWriteFrequency(5, 187500);
  pinMode(DAC_ON, OUTPUT);
  digitalWriteFast(DAC_ON, HIGH);                                               // pull high to power off, pull low to keep power.

// Set pinmodes for the coralspeq
  //pinMode(SPEC_EOS, INPUT);
  pinMode(SPEC_GAIN, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);
  pinMode(SPEC_CLK, OUTPUT);

  digitalWrite(SPEC_GAIN, LOW);
  digitalWrite(SPEC_ST, HIGH);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_GAIN, HIGH); //High Gain
  //digitalWrite(SPEC_GAIN, LOW); //LOW Gain

  float tmp = 0;
  EEPROM_readAnything(1780,tmp);
  if (tmp != (float) DEVICE_FIRMWARE) {                                                // if the current firmware version isn't what's saved in EEPROM memory, then...
    EEPROM_writeAnything(1780,(float) DEVICE_FIRMWARE);                          // save the current firmware version
  }

  MAG3110_init();                                                                // initialize compass and accelerometer
  MMA8653FC_init();


}

void pwr_off() { 
  digitalWriteFast(PWR_OFF, HIGH);
  delay(50);
  digitalWriteFast(PWR_OFF, LOW);
  Serial_Print("{\"pwr_off\":\"HIGH\"}");
  Serial_Print_CRC();
}

void pwr_off_lights() {
  Serial_Print("{\"pwr_off_lights\": \"");
  if (pwr_off_lights_state == 0) {
    Serial_Print("HIGH\"}");
    digitalWriteFast(PWR_OFF_LIGHTS, HIGH);
    pwr_off_lights_state = 1;
  }
  else if (pwr_off_lights_state == 1) {
    Serial_Print("LOW\"}");
    digitalWriteFast(PWR_OFF_LIGHTS, LOW);    
    pwr_off_lights_state = 0;
  }
  Serial_Print_CRC();
}

/*
Battery output:
 batt_level:[bv0,bv1,bv2]
 
 battery data intepretation:
 if bv0 - bv1 < .1 - state: "USB Power Only"
 else if bv1 < 5 && bv0 > 7 - state "Low quality batteries, replace with NiMh or Li"
 else if bv1 < 5 - state "Replace batteries"
 else if bv0 - bv1 > .1, then:
 if bv0 < 6.6 - state "Replace batteries"
 if 6.6 - 7.1 - state "Batteries low"
 if 7.1 - 7.6 - state "Batteries OK"
 if 7.6 + - state "Batteries good"  
 
 Please print both the intepretation and the raw values (we'll want to watch them over time to get more info out of them).
 */

void batt_level(int curly) {
  dac.analogWrite(0,4095);                                        // if we are setting actinic equal to the light sensor, then do it!
  digitalWriteFast(DAC_ON, LOW);                                               // pull high to power off, pull low to keep power.
  delayMicroseconds(1);
  digitalWriteFast(DAC_ON, HIGH);                                               // pull high to power off, pull low to keep power.
  float bv0 = ((float) analogRead(BATT_LEVEL) / 2358);                // read battery voltage in normal low draw state
  digitalWriteFast(ACTINICLIGHT1, HIGH);                                               // pull high to power off, pull low to keep power.
  delay(10);
  float bv1 = ((float) analogRead(BATT_LEVEL) / 2358);               // read battery voltage in start of high draw state
  delay(110);
  float bv2 = ((float) analogRead(BATT_LEVEL) / 2358);               // read battery voltage after 100ms of high draw state
  digitalWriteFast(ACTINICLIGHT1, LOW);                                               // pull high to power off, pull low to keep power.
  if (curly == 1) {
    Serial_Print("{");
  }
  Serial_Print("\"batt_level\": [");
  Serial_Print_Float(bv0,2);
  Serial_Print(",");
  Serial_Print_Float(bv1,2);
  Serial_Print(",");
  Serial_Print_Float(bv2,2);
  Serial_Print("]");
  if (curly == 1) {
    Serial_Print("}");
    Serial_Print_CRC();
  }
}


void get_calibration_userdef() {
  
      Serial_Print("\"get_userdef0\": [");
      Serial_Print_Float(userdef0[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef0[1],6);
      Serial_Print("],");

      Serial_Print("\"get_userdef1\":");
      Serial_Print_Float(userdef1[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef1[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef2\":");
      Serial_Print_Float(userdef2[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef2[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef3\":");
      Serial_Print_Float(userdef3[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef3[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef4\":");
      Serial_Print_Float(userdef4[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef4[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef5\":");
      Serial_Print_Float(userdef5[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef5[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef6\":");
      Serial_Print_Float(userdef6[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef6[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef7\":");
      Serial_Print_Float(userdef7[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef7[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef8\":");
      Serial_Print_Float(userdef8[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef8[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef9\":");
      Serial_Print_Float(userdef9[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef9[1],6);
      Serial_Print("],");
      Serial_Print(",");

      Serial_Print("\"get_userdef10\":");
      Serial_Print_Float(userdef10[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef10[1],6);
      Serial_Print("],");
      Serial_Print(",");
 }    


void get_calibration(float slope[], float yint[],float _slope, float _yint, JsonArray cal,String name) {
  if (cal.getLong(0) > 0) {                                                          // if get_ir_baseline is true, then...    
    Serial_Print("\"");
    Serial_Print(name.c_str());
    Serial_Print("\": [");
    if (_slope != 0) {
      Serial_Print_Float(_slope,6);
      if (yint [0] != 0) {
        Serial_Print(",");
        Serial_Print_Float(_yint,6);                                                  // ignore second value if it's 0
      }
    }
    else if (_slope == 0) {
    for (int z = 0;z<cal.getLength();z++) {                                          // check each pins in the get_ir_baseline array, and for each pin...
      Serial_Print("[");
      Serial_Print_Float(cal.getLong(z),0);                                                  // print the pin name
      Serial_Print(","); 
      for (int i = 0;i<sizeof(all_pins)/sizeof(int);i++) {                          // look through available pins, pull out the baseline slope and yint associated with requested pin
        if (all_pins[i] == cal.getLong(z)) {
          if (_slope == 0) {                                                        // if this is an array, then print this way...
            Serial_Print_Float(slope[i],6);
            if (yint [0] != 0) {                                                        
              Serial_Print(",");
              Serial_Print_Float(yint[i],6);                                                // ignore second value if it's 0
            }
          }
          Serial_Print("]");
          goto cal_end;                                                                            // bail if you found your pin
        }
      }
cal_end:
      delay(1);
      if (z != cal.getLength()-1) {                                                  // add a comma if it's not the last value
        Serial_Print(",");
      }
    }
    }
    Serial_Print("],");
  }
}

// give it a channel, outputs outputs x,y,z 3 axis data comes out (16 bit)
void compass () {}

// give it a channel, outputs outputs x,y,z  3 axis data comes out (16 bit)
void accel () {}

// read this channel, replys with one number (16 bit)
void atod (int channel) {}


void measureit() {
  int i;
  int readit;
  pinMode(A14,INPUT);
  Serial_Print_Float(A14,0);
  for (i=0;i<100;i++) {
    readit = analogRead(A14);
    Serial_Print_Float(readit,0); 
    delay(2000);
  }
  /*
  for (i=0;i<100;i++) {
   pinMode(3,OUTPUT);
   digitalWrite(3, HIGH);
   delay(2000);
   digitalWrite(3, LOW);
   delay(2000);
   }
   */
}

void dacit() {            // 1028

  analogWriteRes(12);
  int i;
//  int readit;
  pinMode(A14,OUTPUT);
  
/*                              // speed testing for various analog read and writes
  delay(5);
  long gogo = 0;
  long endend = 0;
  for (i=0;i<40;i++) {
    gogo = micros();
//    analogWrite(A14,i*100);
//    data1 = adc->analogRead(detector,ADC_0);
//    analogRead(A14);
    endend = micros();
    Serial_Print_Line(endend - gogo);
    delay(100);
  }
//  Serial_Print_Line(A14);
*/

  while (Serial.available() == 0) {
  for (i=0;i<1500;i++) {
    analogWrite(A14,i);
//    Serial_Print_Line(readit); 
    delay(3);
    int j = analogRead(26);
    Serial_Print_Float(i,0);
    Serial_Print(",");
    Serial_Print_Float(j,0);
    Serial_Print(",");
  }
  for (i=1500;i>0;i--) {
    analogWrite(A14,i);
//    Serial_Print_Line(readit); 
    delay(3);
    int j = analogRead(26);
    Serial_Print_Float(i,0);
    Serial_Print(",");
    Serial_Print_Float(j,0);
    Serial_Print(",");
  }
  }
  Serial.read();
  Serial.read();
  /*
  for (i=0;i<100;i++) {
   pinMode(3,OUTPUT);
   digitalWrite(3, HIGH);
   delay(2000);
   digitalWrite(3, LOW);
   delay(2000);
   }
   */
}


//////////////////////// MAIN LOOP /////////////////////////
void loop() {
  
  delay(50);
  int measurements = 1;                                                   // the number of times to repeat the entire measurement (all protocols)
  unsigned long measurements_delay = 0;                                    // number of seconds to wait between measurements  
  unsigned long measurements_delay_ms = 0;                                    // number of milliseconds to wait between measurements  
  volatile unsigned long meas_number = 0;                                       // counter to cycle through measurement lights 1 - 4 during the run
  unsigned long end1;
  unsigned long start1 = millis(); 

  // these variables could be pulled from the JSON at the time of use... however, because pulling from JSON is slow, it's better to create a int to save them into at the beginning of a protocol run and use the int instead of the raw hashTable.getLong type call 
  int _act1_light;
  int _act2_light;
  int _alt1_light;
  int _alt2_light;
  int _act1_light_prev;
  int _act2_light_prev;
  int _alt1_light_prev;
  int _alt2_light_prev;
  int act_background_light_prev = 13;
  int spec;

  int cycle = 0;                                                                // current cycle number (start counting at 0!)
  int pulse = 0;                                                                // current pulse number
  int total_cycles;	                       	                        	// Total number of cycles - note first cycle is cycle 0
  int alt1_state;
  int alt2_state;
  int meas_array_size = 0;                                                      // measures the number of measurement lights in the current cycle (example: for meas_lights = [[15,15,16],[15],[16,16,20]], the meas_array_size's are [3,1,3].  
  char* json = (char*)malloc(1);
  char w;
  char* name;
  JsonHashTable hashTable;
  JsonParser<600> root;
  int number_of_protocols = 0;                                                      // reset number of protocols
  int end_flag = 0;
  int which_serial = 0;
//  long* data_raw_average = (long*)malloc(4);
  unsigned long* data_raw_average = (unsigned long*)malloc(4);
  char serial_buffer [serial_buffer_size];
  String json2 [max_jsons];
  memset(serial_buffer,0,serial_buffer_size);                                    // reset buffer to zero
  for (int i=0;i<max_jsons;i++) {
    json2[i] = "";                                                              // reset all json2 char's to zero (ie reset all protocols)
  }

  call_print_calibration(0);                                                                  // recall all data saved in eeprom

  digitalWriteFast(SAMPLE_AND_HOLD,LOW);                                          // discharge sample and hold in case the cap has be
  delay(10);
  digitalWriteFast(SAMPLE_AND_HOLD,HIGH);

  String choose = "0";
  while (Serial.peek() != '[' && Serial1.peek() != '[') {                      // wait till we see a "[" to start the JSON of JSONS
    choose = user_enter_str(50,1);

    if (choose.toInt() > 0 && choose.toInt() < 200) {                    // if it's 0 - 200 then go see full light of device testing
      lighttests(choose.toInt());
      serial_bt_flush();                                                             // flush serial port if it's none of the above things
    }
    else {
      switch (choose.toInt()) {
      case false:                                                                   // if it's not a number, then exit switch and check to see if it's a protocol
        break;
      case -1:
        delay(10);
        break;
      case 1000:                                                                    // print "Ready" to USB and Bluetooth
        Serial_Print_Line(" Ready");
        break;
      case 1001:                                                                    // power off completely (if USB connected, only batteries power down
        pwr_off();
        break;
      case 1002:                                                                    // power off completely (if USB connected, only batteries power down
        dac.analogWrite(0,400);
//        dac.analogWrite(3,2000);
        digitalWriteFast(DAC_ON, LOW);                                                       // and turn it on.
        delayMicroseconds(1);                                                     
        digitalWriteFast(DAC_ON, HIGH);                                              
        delay(4);
        digitalWriteFast(MEASURINGLIGHT1, HIGH);
        delay(1000);
        digitalWriteFast(MEASURINGLIGHT1, LOW);
        Serial.print("here1");
        digitalWriteFast(MEASURINGLIGHT2, HIGH);
        delay(1000);
        digitalWriteFast(MEASURINGLIGHT2, LOW);
        Serial.print("here2");
        digitalWriteFast(ACTINICLIGHT1, HIGH);
        delay(1000);
        digitalWriteFast(ACTINICLIGHT1, LOW);
        Serial.print("here3");
        dac.analogWrite(0,0);
//        dac.analogWrite(3,0);
        digitalWriteFast(DAC_ON, LOW);                                                       // and turn it on.
        delayMicroseconds(1);                                                     
        digitalWriteFast(DAC_ON, HIGH);                                              
        break;    
      case 1003:                                                                    // power down lights (TL1963) only
        pwr_off_lights();
        break;
      case 1004:                                                                    // show battery level and associated values
        batt_level(1);
        break;
      case 1005:                                                                    // print all calibration data
        call_print_calibration(1);
        break;
      case 1006:                                                                    // calibrate tcs light sensor to actual PAR values
        calibrate_light_sensor();
        break;
      case 1007:                                                                   // view device info  
        set_device_info(0);                                                       
        break;
      case 1008:                                                                    // calibrate offset
        calibrate_offset();
        break;
      case 1011:                                                                    // calibrate the lights (eeprom 60 - 300)
        add_calibration(60);
        break;
      case 1012:                                                                    // factory calibration of lights (eeprom 300 - 540)
        add_calibration(300);
        break;
      case 1013:                                                                    // view and set device info      
        set_device_info(1);                                                                                                            
        break;
      case 1014:                                                                    // calibrate the baseline  (eeprom 540 - 880) 
        add_calibration(540);
        break;
      case 1015:                                                                   // calibrate the spad blanks  (eeprom 880 - 1120)
        add_calibration(880);
        break;
      case 1016:                                                                   // calibrate the other user defined calibrations  (eeprom 1120 - 1360)
        add_calibration(1120);
        break;
      case 1017:                                                                   // reset all saved EEPROM values to zero
        reset_all(0);
        break;
      case 1018:                                                                   // reset only the calibration arrays to zero (leave tcs calibration, offset, device info)
        reset_all(1);
        break;
      case 1019:                                                                   // save spad_factor value to EEPROM
        add_userdef(40,2,4,1);
        break;
      case 1020:                                                                   // save user defined value to EEPROM
        add_userdef(1360,2,4,1);
        break;
      case 1021:                                                                   // save user defined value to EEPROM
        add_userdef(1368,2,4,1);
        break;
      case 1022:                                                                   // save user defined value to EEPROM
        add_userdef(1376,2,4,1);
        break;
      case 1023:                                                                   // save user defined value to EEPROM
        add_userdef(1384,2,4,1);
        break;
      case 1024:                                                                   // save user defined value to EEPROM
        add_userdef(1392,2,4,1);
        break;
      case 1025:                                                                   // save user defined value to EEPROM
        add_userdef(1400,2,4,1);
        break;
      case 1026:
        measureit();
        break;
      case 1027:
        _reboot_Teensyduino_();                                                    // restart teensy
        break;
      case 1028:
        dacit();
        break;
      case 1029:                                                                   // save user defined value to EEPROM
        add_userdef(1408,2,4,1);
        break;
      case 1030:                                                                   // save user defined value to EEPROM
        add_userdef(1416,2,4,1);
        break;
      case 1031:                                                                   // save user defined value to EEPROM
        add_userdef(1424,2,4,1);
        break;
      case 1032:                                                                   // save user defined value to EEPROM
        add_userdef(1432,2,4,1);
        break;
      case 1033:                                                                   // save user defined value to EEPROM for how many ms to delay before batteries shutoff (pwr_off_ms)
        add_userdef(1440,1,4,0);
        Serial_Print("{");
        EEPROM_readAnything(1440,pwr_off_ms);
        print_cal_userdef("pwr_off_ms", pwr_off_ms,1,2);
        Serial_Print_CRC();  
        break;
      case 1034:                                                                   // save user defined value to EEPROM
        add_userdef(1448,2,4,1);
        break;
      case 1035:                                                                   // save user defined value to EEPROM
        add_userdef(1456,2,4,1);
        break;
      case 1036:                                                                   // save user defined value to EEPROM
        add_userdef(1464,2,4,1);
        break;
      case 1037:                                                                   // save user defined value to EEPROM
        add_userdef(1760,2,4,1);
        break;
      case 1038:                                                                   // save user defined value to accelerometer
          int Xval, Yval, Zval;
          while (Serial.peek() != '1') {
            MMA8653FC_read(&Xval,&Yval,&Zval);
            Serial_Print_Float(Xval,6);
            Serial_Print(",");
            Serial_Print_Float(Yval,6);
            Serial_Print(",");
            Serial_Print_Float(Zval,6);
            Serial_Print(",");
            delay(100);
          }
          Serial.read();
          break;
      case 1039:                                                                   // save user defined value to compass
          int Xcomp, Ycomp, Zcomp;
          while (Serial1.peek() != '1') {
            MAG3110_read(&Xcomp,&Ycomp,&Zcomp);
            delay(200);
            Serial_Print_Float(Xcomp,6);
            Serial_Print(",");
            Serial_Print_Float(Ycomp,6);
            Serial_Print(",");
            Serial_Print_Float(Zcomp,6);
            Serial_Print(",");
          }
          Serial.read();
          break;
      case 1040:                                                                   // save user defined value to EEPROM
//          Xval, Yval, Zval;
//          Xcomp, Ycomp, Zcomp;
          while (Serial.peek() != '1') {
            MMA8653FC_read(&Xval,&Yval,&Zval);
            MAG3110_read(&Xcomp,&Ycomp,&Zcomp);
            delay(200);
            Serial_Print_Float(Xval,6);
            Serial_Print(",");
            Serial_Print_Float(Yval,6);
            Serial_Print(",");
            Serial_Print_Float(Zval,6);
            Serial_Print(",");
            Serial_Print_Float(Xcomp,6);
            Serial_Print(",");
            Serial_Print_Float(Ycomp,6);
            Serial_Print(",");
            Serial_Print_Float(Zcomp,6);
            Serial_Print(",");
          }
          Serial.read();
          break;
      case 1041:                                                                   // save user defined value to EEPROM
        add_userdef(1472,2,4,1);
        break;
      case 1042:                                                                   // save user defined value to EEPROM
        add_userdef(1480,2,4,1);
        break;
      case 1043:                                                                   // save user defined value to EEPROM
        add_userdef(1488,2,4,1);
        break;
      case 1044:                                                                   // save user defined value to EEPROM
        add_userdef(1496,2,4,1);
        break;
      case 1045:                                                                   // save user defined value to EEPROM
        add_userdef(1504,2,4,1);
        break;
      case 1046:                                                                   // save user defined value to EEPROM
        add_userdef(1512,2,4,1);
        break;
      case 1047:                                                                   // save user defined value to EEPROM
        add_userdef(1520,2,4,1);
        break;
      case 1048:                                                                   // save user defined value to EEPROM
        add_userdef(1528,2,4,1);
        break;
      case 1049:                                                                   // save user defined value to EEPROM
        add_userdef(1536,2,4,1);
        break;
      case 1050:                                                                   // save user defined value to EEPROM
        add_userdef(1544,2,4,1);
        break;
      case 1051:                                                                   // save user defined value to EEPROM
        add_userdef(1552,2,4,1);
        break;
      case 1052:                                                                   // save user defined value to EEPROM
        add_userdef(1560,2,4,1);
        break;
      case 1053:                                                                   // save user defined value to EEPROM
        add_userdef(1568,2,4,1);
        break;
      case 1054:                                                                   // save user defined value to EEPROM
        add_userdef(1576,2,4,1);
        break;
      case 1055:                                                                   // save user defined value to EEPROM
        add_userdef(1592,2,4,1);
        break;
      case 1056:                                                                   // save user defined value to EEPROM
        add_userdef(1600,2,4,1);
        break;
      case 1057:                                                                   // save user defined value to EEPROM
        add_userdef(1608,2,4,1);
        break;
      case 1058:                                                                   // save user defined value to EEPROM
        add_userdef(1616,2,4,1);
        break;
      case 1059:                                                                   // save user defined value to EEPROM
        add_userdef(1624,2,4,1);
        break;
      case 1060:                                                                   // save user defined value to EEPROM
        add_userdef(1632,2,4,1);
        break;
      case 1061:                                                                   // save user defined value to EEPROM
        add_userdef(1640,2,4,1);
        break;
      case 1062:                                                                   // save user defined value to EEPROM
        add_userdef(1648,2,4,1);
        break;
      case 1063:                                                                   // save user defined value to EEPROM
        add_userdef(1656,2,4,1);
        break;
      case 1064:                                                                   // save user defined value to EEPROM
        add_userdef(1664,2,4,1);
        break;
      case 1065:                                                                   // save user defined value to EEPROM
        add_userdef(1672,2,4,1);
        break;
      case 1066:                                                                   // save user defined value to EEPROM
        add_userdef(1680,2,4,1);
        break;
      case 1067:                                                                   // save user defined value to EEPROM
        add_userdef(1688,2,4,1);
        break;
      case 1068:                                                                   // save user defined value to EEPROM
        add_userdef(1696,2,4,1);
        break;
      case 1069:                                                                   // save user defined value to EEPROM
        add_userdef(1704,2,4,1);
        break;
      case 1070:                                                                   // save user defined value to EEPROM
        add_userdef(1712,2,4,1);
        break;
      case 1071:                                                                   // save user defined value to EEPROM
        add_userdef(1720,3,4,1);
        break;
      case 1072:                                                                   // save user defined value to EEPROM
        add_userdef(1732,3,4,1);
        break;
      case 1073:                                                                   // save user defined value to EEPROM
        add_userdef(1744,3,4,1);
        break;
      case 1074:                                                                   // save user defined value to EEPROM
        add_userdef(1756,3,4,1);
        break;
      case 1075:                                                                   // save user defined value to EEPROM
        add_userdef(1768,3,4,1);
        break;
      case 1076:                                                                   // save user defined value to EEPROM        
        Serial_Print_Float(MLX90614_Read(0),3);
        break;
      case 1077:                                                                   // read spectrophotometer mode     
        readSpectrometer();
        print_data();
        break;
      case 1078:                                                                   // over the air update of firmware.    
        upgrade_firmware();
        break;
        }
     }
  }        
  if (Serial.peek() == 91) {
#ifdef DEBUGSIMPLE
    Serial_Print_Line("computer serial");
#endif
    which_serial = 1;
  }
  else if (Serial1.peek() == 91) {
#ifdef DEBUGSIMPLE
    Serial_Print_Line("bluetooth serial");
#endif
    which_serial = 2;
  } 

  Serial.read();                                       // flush the "["
  Serial1.read();                                       // flush the "["

  switch (which_serial) {
  case 0:
    Serial_Print_Line("error in choosing bluetooth or serial");
    break;

  case 1:
    Serial.setTimeout(2000);                                          // make sure set timeout is 1 second
    Serial.readBytesUntil('!',serial_buffer,serial_buffer_size);
    for (int i=0;i<5000;i++) {                                    // increments through each char in incoming transmission - if it's open curly, it saves all chars until closed curly.  Any other char is ignored.
      if (serial_buffer[i] == '{') {                               // wait until you see a open curly bracket
        while (serial_buffer[i] != '}') {                          // once you see it, save incoming data to json2 until you see closed curly bracket
          json2[number_of_protocols] +=serial_buffer[i];
          i++;
        }
        json2[number_of_protocols] +=serial_buffer[i];            // catch the last closed curly
        number_of_protocols++;
      }        
    }  
    for (int i=0;i<number_of_protocols;i++) {
      if (json2[i] != '0') {
#ifdef DEBUGSIMPLE
        Serial_Print("Incoming JSON as received by Teensy:   ");
        Serial_Print_Line(json2[i]);
#endif
      }
    }
    break;

  case 2:
    Serial1.setTimeout(1000);                                          // make sure set timeout is 1 second
    Serial1.readBytesUntil('!',serial_buffer,serial_buffer_size);
    for (int i=0;i<5000;i++) {                                    // increments through each char in incoming transmission - if it's open curly, it saves all chars until closed curly.  Any other char is ignored.
      if (serial_buffer[i] == '{') {                               // wait until you see a open curly bracket
        while (serial_buffer[i] != '}') {                          // once you see it, save incoming data to json2 until you see closed curly bracket
          json2[number_of_protocols] +=serial_buffer[i];
          i++;
        }
        json2[number_of_protocols] +=serial_buffer[i];            // catch the last closed curly
        number_of_protocols++;
      }        
    }  

    for (int i=0;i<number_of_protocols;i++) {
      if (json2[i] != '0') {
#ifdef DEBUGSIMPLE
        Serial_Print("Incoming JSON as received by Teensy:   ");
        Serial_Print_Line(json2[i]);
#endif
      }
    }
    break;
  }

  Serial_Print("{\"device_name\":\"");
  Serial_Print(DEVICE_NAME);
  Serial_Print("\",\"device_id\":");
  Serial_Print_Float(device_id,0);
  Serial_Print(",\"device_version\":");
  Serial_Print_Float(device_version,2);
  Serial_Print(",\"device_firmware\":\"");
  Serial_Print_Float(DEVICE_FIRMWARE,2);
  Serial_Print("\",\"sample\":[");

  for (int y=0;y<measurements;y++) {                                                              // loop through the all measurements to create a measurement group

    for (int q = 0;q<number_of_protocols;q++) {                                               // loop through all of the protocols to create a measurement

      free(json);                                                                             // free initial json malloc, make sure this is here! Free before resetting the size according to the serial input
      json = (char*)malloc((json2[q].length()+1)*sizeof(char));
      strncpy(json,json2[q].c_str(),json2[q].length());  
      json[json2[q].length()] = '\0';                                                         // Add closing character to char*
      hashTable = root.parseHashTable(json);
      if (!hashTable.success()) {                                                             // NOTE: if the incomign JSON is too long (>~5000 bytes) this tends to be where you see failure (no response from device)
        Serial_Print("{\"error\":\"JSON not recognized, or other failure with Json Parser\"}, the data JSON we received is: ");
        for (int i=0;i<5000;i++) {
          Serial_Print(json2[i].c_str());
        }
        serial_bt_flush();
        return;
      }

      int protocols = 1;
      int quit = 0;
      for (int u = 0;u<protocols;u++) {                                                        // the number of times to repeat the current protocol
      
        String protocol_id =      hashTable.getString("protocol_id");                          // used to determine what macro to apply
        int analog_averages =     hashTable.getLong("analog_averages");                          // DEPRECIATED IN NEWEST HARDWARE 10/14 # of measurements per measurement pulse to be internally averaged (min 1 measurement per 6us pulselengthon) - LEAVE THIS AT 1 for now
        if (analog_averages == 0) {                                                              // if averages don't exist, set it to 1 automatically.
          analog_averages = 1;
        }
        averages =                hashTable.getLong("averages");                               // The number of times to average this protocol.  The spectroscopic and environmental data is averaged in the device and appears as a single measurement.
        if (averages == 0) {                                                                   // if averages don't exist, set it to 1 automatically.
          averages = 1;
        }
        int averages_delay =      hashTable.getLong("averages_delay");
        int averages_delay_ms =   hashTable.getLong("averages_delay_ms");                       // same as above but in ms
        measurements =            hashTable.getLong("measurements");                            // number of times to repeat a measurement, which is a set of protocols
        measurements_delay =      hashTable.getLong("measurements_delay");                      // delay between measurements in seconds
        measurements_delay_ms =      hashTable.getLong("measurements_delay_ms");                      // delay between measurements in milliseconds
        protocols =               hashTable.getLong("protocols");                               // delay between protocols within a measurement
        if (protocols == 0) {                                                                   // if averages don't exist, set it to 1 automatically.
          protocols = 1;
        }
        int protocols_delay =     hashTable.getLong("protocols_delay");                         // delay between protocols within a measurement
        int protocols_delay_ms =     hashTable.getLong("protocols_delay_ms");                         // delay between protocols within a measurement in milliseconds
        if (hashTable.getLong("act_background_light") == 0) {                                    // The Teensy pin # to associate with the background actinic light.  This light continues to be turned on EVEN BETWEEN PROTOCOLS AND MEASUREMENTS.  It is always Teensy pin 13 by default.
          act_background_light =  13;                                                            // change to new background actinic light
        }
        else {
          act_background_light =  hashTable.getLong("act_background_light");
        }
/*
averaging0 - 1 - 30
averaging1 - 1 - 30
resolution0 - 2 - 16
resolution1 - 2 - 16
conversion_speed - 0 - 5
sampling_speed - 0 - 5
*/

        int averaging0 =          hashTable.getLong("averaging");                               // # of ADC internal averages
        if (averaging0 == 0) {                                                                   // if averaging0 don't exist, set it to 10 automatically.
          averaging0 = 10;
        }
        int averaging1 = averaging0;
        int resolution0 =         hashTable.getLong("resolution");                               // adc resolution (# of bits)
        if (resolution0 == 0) {                                                                   // if resolution0 don't exist, set it to 16 automatically.
          resolution0 = 16;
        }
        int resolution1 = resolution0;
        int conversion_speed =    hashTable.getLong("conversion_speed");                               // ADC speed to convert analog to digital signal (5 fast, 0 slow)
        if (conversion_speed == 0) {                                                                   // if conversion_speed don't exist, set it to 3 automatically.
          conversion_speed = 3;
        }
        int sampling_speed =      hashTable.getLong("sampling_speed");                               // ADC speed of sampling (5 fast, 0 slow)
        if (sampling_speed == 0) {                                                                   // if sampling_speed don't exist, set it to 3 automatically.
          sampling_speed = 3;
        }
        int act_background_light_intensity = hashTable.getLong("act_background_light_intensity");  // sets intensity of background actinic.  Choose this OR tcs_to_act.
        int tcs_to_act =          hashTable.getLong("tcs_to_act");                               // sets the % of response from the tcs light sensor to act as actinic during the run (values 1 - 100).  If tcs_to_act is not defined (ie == 0), then the act_background_light intensity is set to actintensity1.
        long pulsesize =          hashTable.getLong("pulsesize");                                // Size of the measuring pulse (5 - 100us).  This also acts as gain control setting - shorter pulse, small signal. Longer pulse, larger signal.  
        long pulsedistance =      hashTable.getLong("pulsedistance");                            // distance between measuring pulses in us.  Minimum 1000 us.
        int offset_off =          hashTable.getLong("offset_off");                               // turn off detector offsets (default == 0 which is on, set == 1 to turn offsets off)
        int get_offset =          hashTable.getLong("get_offset");                               // include detector offset information in the output
        // NOTE: it takes about 50us to set a DAC channel via I2C at 2.4Mz.  
        JsonArray get_ir_baseline=hashTable.getArray("get_ir_baseline");                        // include the ir_baseline information from the device for the specified pins
        JsonArray get_tcs_cal =   hashTable.getArray("get_tcs_cal");                            // include the get_tcs_cal information from the device for the specified pins
        JsonArray get_lights_cal= hashTable.getArray("get_lights_cal");                         // include get_lights_cal information from the device for the specified pins
        JsonArray get_blank_cal = hashTable.getArray("get_blank_cal");                          // include the get_blank_cal information from the device for the specified pins
        JsonArray get_other_cal = hashTable.getArray("get_other_cal");                        // include the get_other_cal information from the device for the specified pins
        int get_userdef0 =  hashTable.getLong("get_userdef0");                        // include the saved userdef0 information from the device
        int get_userdef1 =  hashTable.getLong("get_userdef1");                        // include the saved userdef1 information from the device
        int get_userdef2 =  hashTable.getLong("get_userdef2");                        // include the saved userdef2 information from the device
        int get_userdef3 =  hashTable.getLong("get_userdef3");                        // include the saved userdef3 information from the device
        int get_userdef4 =  hashTable.getLong("get_userdef4");                        // include the saved userdef4 information from the device
        int get_userdef5 =  hashTable.getLong("get_userdef5");                        // include the saved userdef5 information from the device
        int get_userdef6 =  hashTable.getLong("get_userdef6");                        // include the saved userdef6 information from the device
        int get_userdef7 =  hashTable.getLong("get_userdef7");                        // include the saved userdef7 information from the device
        int get_userdef8 =  hashTable.getLong("get_userdef8");                        // include the saved userdef8 information from the device
        int get_userdef9 =  hashTable.getLong("get_userdef9");                        // include the saved userdef9 information from the device
        int get_userdef10 =  hashTable.getLong("get_userdef10");                        // include the saved userdef10 information from the device
        int get_userdef11 =  hashTable.getLong("get_userdef11");                        // include the saved userdef11 information from the device
        int get_userdef12 =  hashTable.getLong("get_userdef12");                        // include the saved userdef12 information from the device
        int get_userdef13 =  hashTable.getLong("get_userdef13");                        // include the saved userdef13 information from the device
        int get_userdef14 =  hashTable.getLong("get_userdef14");                        // include the saved userdef14 information from the device
        int get_userdef15 =  hashTable.getLong("get_userdef15");                        // include the saved userdef15 information from the device
        int get_userdef16 =  hashTable.getLong("get_userdef16");                        // include the saved userdef16 information from the device
        int get_userdef17 =  hashTable.getLong("get_userdef17");                        // include the saved userdef17 information from the device
        int get_userdef18 =  hashTable.getLong("get_userdef18");                        // include the saved userdef18 information from the device
        int get_userdef19 =  hashTable.getLong("get_userdef19");                        // include the saved userdef19 information from the device
        int get_userdef20 =  hashTable.getLong("get_userdef20");                        // include the saved userdef20 information from the device
        int get_userdef21 =  hashTable.getLong("get_userdef21");                        // include the saved userdef21 information from the device
        int get_userdef22 =  hashTable.getLong("get_userdef22");                        // include the saved userdef22 information from the device
        int get_userdef23 =  hashTable.getLong("get_userdef23");                        // include the saved userdef23 information from the device
        int get_userdef24 =  hashTable.getLong("get_userdef24");                        // include the saved userdef24 information from the device
        int get_userdef25 =  hashTable.getLong("get_userdef25");                        // include the saved userdef25 information from the device
        int get_userdef26 =  hashTable.getLong("get_userdef26");                        // include the saved userdef26 information from the device
        int get_userdef27 =  hashTable.getLong("get_userdef27");                        // include the saved userdef27 information from the device
        int get_userdef28 =  hashTable.getLong("get_userdef28");                        // include the saved userdef28 information from the device
        int get_userdef29 =  hashTable.getLong("get_userdef29");                        // include the saved userdef29 information from the device
        int get_userdef30 =  hashTable.getLong("get_userdef30");                        // include the saved userdef30 information from the device
        int get_userdef31 =  hashTable.getLong("get_userdef31");                        // include the saved userdef31 information from the device
        int get_userdef32 =  hashTable.getLong("get_userdef32");                        // include the saved userdef32 information from the device
        int get_userdef33 =  hashTable.getLong("get_userdef33");                        // include the saved userdef33 information from the device
        int get_userdef34 =  hashTable.getLong("get_userdef34");                        // include the saved userdef34 information from the device
        int get_userdef35 =  hashTable.getLong("get_userdef35");                        // include the saved userdef35 information from the device
        int get_userdef36 =  hashTable.getLong("get_userdef36");                        // include the saved userdef36 information from the device
        int get_userdef37 =  hashTable.getLong("get_userdef37");                        // include the saved userdef37 information from the device
        int get_userdef38 =  hashTable.getLong("get_userdef38");                        // include the saved userdef38 information from the device
        int get_userdef39 =  hashTable.getLong("get_userdef39");                        // include the saved userdef39 information from the device
        int get_userdef40 =  hashTable.getLong("get_userdef40");                        // include the saved userdef40 information from the device
        int get_userdef41 =  hashTable.getLong("get_userdef41");                        // include the saved userdef41 information from the device
        int get_userdef42 =  hashTable.getLong("get_userdef42");                        // include the saved userdef42 information from the device
        int get_userdef43 =  hashTable.getLong("get_userdef43");                        // include the saved userdef43 information from the device
        int get_userdef44 =  hashTable.getLong("get_userdef44");                        // include the saved userdef44 information from the device
        int get_userdef45 =  hashTable.getLong("get_userdef45");                        // include the saved userdef45 information from the device
        int get_userdef46 =  hashTable.getLong("get_userdef46");                        // include the saved userdef46 information from the device
        int get_userdef47 =  hashTable.getLong("get_userdef47");                        // include the saved userdef47 information from the device
        int get_userdef48 =  hashTable.getLong("get_userdef48");                        // include the saved userdef48 information from the device
        int get_userdef49 =  hashTable.getLong("get_userdef49");                        // include the saved userdef49 information from the device
        int get_userdef50 =  hashTable.getLong("get_userdef50");                        // include the saved userdef50 information from the device
        JsonArray pulses =        hashTable.getArray("pulses");                                // the number of measuring pulses, as an array.  For example [50,10,50] means 50 pulses, followed by 10 pulses, follwed by 50 pulses.
        JsonArray act1_lights =   hashTable.getArray("act1_lights");
        JsonArray act2_lights =   hashTable.getArray("act2_lights");
        JsonArray alt1_lights =   hashTable.getArray("alt1_lights");
        JsonArray alt2_lights =   hashTable.getArray("alt2_lights");
        act_intensities =         hashTable.getArray("act_intensities");                         // write to input register of a DAC. channel 0 for low (actinic).  1 step = +3.69uE (271 == 1000uE, 135 == 500uE, 27 == 100uE)
        meas_intensities =        hashTable.getArray("meas_intensities");                        // write to input register of a DAC. channel 3 measuring light.  0 (high) - 4095 (low).  2092 = 0.  From 2092 to zero, 1 step = +.2611uE
        cal_intensities =         hashTable.getArray("cal_intensities");                         // write to input register of a DAC. channel 2 calibrating light.  0 (low) - 4095 (high).
        JsonArray detectors =     hashTable.getArray("detectors");                               // the Teensy pin # of the detectors used during those pulses, as an array of array.  For example, if pulses = [5,2] and detectors = [[34,35],[34,35]] .  
        JsonArray meas_lights =   hashTable.getArray("meas_lights");
        JsonArray environmental = hashTable.getArray("environmental");
//        JsonArray message_type =  hashTable.getArray("message_type");                                // sends the user a message which they must reply -1+ to continue
        JsonArray message =       hashTable.getArray("message");                                // sends the user a message to which they must reply <answer>+ to continue
        
        //********************INPUT DATA FOR CORALSPEQ********************//
        JsonArray spec =          hashTable.getArray("spec");                                // defines whether the spec will be called during each array.  note for each single plus, the spec will call and add 256 values to data_raw!
        int _delay_time = hashTable.getLong("delay_time");                                         // delay per half clock (in microseconds).  This ultimately conrols the integration time.
        if (_delay_time == 0) {                                                                    // if user inputted a value, then save it.  Otherwise, use default value
        delay_time = 1;
        }
        int _read_time = hashTable.getLong("read_time");                                        // Amount of time that the analogRead() procedure takes (in microseconds)
        if (read_time == 0) {    
        read_time = 35;        
        }
        int _intTime = hashTable.getLong("intTime");                                         // delay per half clock (in microseconds).  This ultimately conrols the integration time.
        if (intTime == 0) { 
        intTime = 100;
        }
        int _accumulateMode = hashTable.getLong("accumulateMode");
        if (accumulateMode == 0) { 
        accumulateMode = false;
        }
        
        total_cycles =            pulses.getLength()-1;                                          // (start counting at 0!)

        long size_of_data_raw = 0;
        long total_pulses = 0;      
        for (int i=0;i<pulses.getLength();i++) {                                            // count the number of non zero lights and total pulses
          total_pulses += pulses.getLong(i) * meas_lights.getArray(i).getLength();          // count the total number of pulses
          int non_zero_lights = 0;
          for (int j=0;j<meas_lights.getArray(i).getLength();j++) {                         // count the total number of non zero pulses
            if (meas_lights.getArray(i).getLong(j) > 0) {
              non_zero_lights++;
            }
          } //**//
          // redefine the size of data raw to account for the 256 spec measurements per 1 pulse if spec is used (for coralspeq)
          if (spec.getLong(i) == 1) {
            size_of_data_raw += pulses.getLong(i) * non_zero_lights * 256;
          }
          else {
            size_of_data_raw += pulses.getLong(i) * non_zero_lights;
          }
        }
        free(data_raw_average);                                                            // free malloc of data_raw_average
//        data_raw_average = (long*)calloc(size_of_data_raw,sizeof(long));                   // get some memory space for data_raw_average, initialize all at zero.
        data_raw_average = (unsigned long*)calloc(size_of_data_raw,sizeof(unsigned long));                   // get some memory space for data_raw_average, initialize all at zero.

#ifdef DEBUGSIMPLE
        Serial_Print_Line("");
        Serial_Print("size of data raw:  ");
        Serial_Print_Line(size_of_data_raw);

        Serial_Print_Line("");
        Serial_Print("total number of pulses:  ");
        Serial_Print_Line(total_pulses);

        Serial_Print_Line("");
        Serial_Print("all data in data_raw_average:  ");
        for (int i=0;i<size_of_data_raw;i++) {
          Serial_Print(data_raw_average[i]);
        }
        Serial_Print_Line("");

        Serial_Print_Line("");
        Serial_Print("number of pulses:  ");
        Serial_Print_Line(pulses.getLength());

        Serial_Print_Line("");
        Serial_Print("arrays in meas_lights:  ");
        Serial_Print_Line(meas_lights.getLength());

        Serial_Print_Line("");
        Serial_Print("length of meas_lights arrays:  ");
        for (int i=0;i<meas_lights.getLength();i++) {
          Serial_Print(meas_lights.getArray(i).getLength());
          Serial_Print(", ");
        }
        Serial_Print_Line("");
#endif
        Serial_Print("{");

        Serial_Print("\"protocol_id\":\"");
        Serial_Print(protocol_id.c_str());
        Serial_Print("\",");

        if (get_offset == 1) {
          print_offset(1);
        }

        print_get_userdef0(get_userdef0,get_userdef1,get_userdef2,get_userdef3,get_userdef4,get_userdef5,get_userdef6,get_userdef7,get_userdef8,get_userdef9,get_userdef10,get_userdef11,get_userdef12,get_userdef13,get_userdef14,get_userdef15,get_userdef16,get_userdef17,get_userdef18,get_userdef19,get_userdef20,get_userdef21,get_userdef22,get_userdef23,get_userdef24,get_userdef25,get_userdef26,get_userdef27,get_userdef28,get_userdef29,get_userdef30,get_userdef31,get_userdef32,get_userdef33,get_userdef34,get_userdef35,get_userdef36,get_userdef37,get_userdef38,get_userdef39,get_userdef40,get_userdef41,get_userdef42,get_userdef43,get_userdef44,get_userdef45,get_userdef46,get_userdef47,get_userdef48,get_userdef49,get_userdef50); // check to see if we need to print any of the user defined calibrations

        get_calibration(calibration_baseline_slope,calibration_baseline_yint,0,0,get_ir_baseline ,"get_ir_baseline");
        get_calibration(calibration_slope,calibration_yint,0,0,get_lights_cal ,"get_lights_cal");
        get_calibration(calibration_blank1,calibration_blank2,0,0,get_blank_cal,"get_blank_cal");
        get_calibration(calibration_other1,calibration_other2,0,0,get_other_cal,"get_other_cal");
        get_calibration(0,0,light_slope,light_y_intercept,get_tcs_cal,"get_tcs_cal");
//        get_calibration_userdef();

        if (averages > 1) {      
          Serial_Print("\"averages\":"); 
          Serial_Print_Float(averages,0); 
          Serial_Print(","); 
        }

        //        print_sensor_calibration(1);                                               // print sensor calibration data

        // this should be an array, so I can reset it all......
        analog_read_average = 0;
        digital_read_average = 0;
        relative_humidity_average = 0;                                                                    // reset all of the environmental variables
        temperature_average = 0;
        objt_average = 0;
        co2_value_average = 0;
        lux_average = 0;
        r_average = 0;
        g_average = 0;
        b_average = 0;
        lux_average_forpar = 0;
        r_average_forpar = 0;
        g_average_forpar = 0;
        b_average_forpar = 0;
        calculate_offset(pulsesize);                                                                    // calculate the offset, based on the pulsesize and the calibration values (ax+b)

#ifdef DEBUGSIMPLE
        Serial_Print_Line("");
        Serial_Print("\"offsets\": "); 
        Serial_Print(offset_34);
        Serial_Print(",");
        Serial_Print_Line(offset_35);
#endif

        for (int x=0;x<averages;x++) {                                                       // Repeat the protocol this many times  

//            if (user_enter_long(1) == -1) {
//              q = number_of_protocols;
//              y = measurements;
//              u = protocols;
//              x = averages;
//            }

          int background_on = 0;
          long data_count = 0;
          int message_flag = 0;                                                              // flags to indicate if an alert, prompt, or confirm have been called at least once (to print the object name to data JSON)
          lux_local = 0;                                                                     // reset local (this measurement) light levels
          r_local = 0;
          g_local = 0;
          b_local = 0;



          /*
      options for relative humidity, temperature, contactless temperature. light_intensity,co2
           0 - take before spectroscopy measurements
           1 - take after spectroscopy measurements
           */

          for (int i=0;i<environmental.getLength();i++) {                                         // call environmental measurements
#ifdef DEBUGSIMPLE
            Serial_Print_Line("Here's the environmental measurements called:    ");
            Serial_Print(environmental.getArray(i).getString(0));
            Serial_Print(", ");
            Serial_Print_Line(environmental.getArray(i).getLong(1));
#endif

            if (environmental.getArray(i).getLong(1) == 0 \                                    
            && (String) environmental.getArray(i).getString(0) == "relative_humidity") {
              Relative_Humidity((int) environmental.getArray(i).getLong(1));                        // if this string is in the JSON and the 2nd component in the array is == 0 (meaning they want this measurement taken prior to the spectroscopic measurement), then call the associated measurement (and so on for all if statements in this for loop)
              if (x == averages-1) {                                                                // if it's the last measurement to average, then print the results
                Serial_Print("\"relative_humidity\":");
                Serial_Print_Float(relative_humidity_average,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "temperature") {
              Temperature((int) environmental.getArray(i).getLong(1));
              if (x == averages-1) {
                Serial_Print("\"temperature\":");
                Serial_Print_Float(temperature_average,2);  
                Serial_Print(",");     
              }  
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "contactless_temperature") {
              Contactless_Temperature( environmental.getArray(i).getLong(1));
              float c_temp = MLX90614_Read(0);
              if (x == averages-1) {
                Serial_Print("\"contactless_temperature\":");
                Serial_Print_Float(c_temp,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "co2") {
              Co2( environmental.getArray(i).getLong(1));
              if (x == averages-1) {                                                                // if it's the last measurement to average, then print the results
                Serial_Print("\"co2\":");
                Serial_Print_Float(co2_value_average,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "light_intensity") {
              Light_Intensity(1);
              if (x == averages-1) {                
                Serial_Print("\"light_intensity\":");
                Serial_Print_Float(lux_to_uE(lux_average_forpar),2);  
                Serial_Print(",");                
                Serial_Print("\"r\":");
                Serial_Print_Float(lux_to_uE(r_average_forpar),2);  
                Serial_Print(",");  
                Serial_Print("\"g\":");
                Serial_Print_Float(lux_to_uE(g_average_forpar),2);  
                Serial_Print(",");  
                Serial_Print("\"b\":");
                Serial_Print_Float(lux_to_uE(b_average_forpar),2);  
                Serial_Print(",");  
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "light_intensity_raw") {
              Light_Intensity(0);
              if (x == averages-1) {
                Serial_Print("\"light_intensity_raw\":");
                Serial_Print_Float(lux_average,2);  
                Serial_Print(",");                
                Serial_Print("\"r\":");
                Serial_Print_Float(r_average,2);  
                Serial_Print(",");  
                Serial_Print("\"g\":");
                Serial_Print_Float(g_average,2);  
                Serial_Print(",");  
                Serial_Print("\"b\":");
                Serial_Print_Float(b_average,2);  
                Serial_Print(",");  
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "analog_read") {                      // perform analog reads
              int pin = environmental.getArray(i).getLong(2);
              pinMode(pin,INPUT);
              int analog_read = analogRead(pin);
              if (x == averages-1) {
                Serial_Print("\"analog_read\":");
                Serial_Print_Float(analog_read,4);  
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "compass") {                      // perform analog reads
              int Xcomp, Ycomp, Zcomp;
              MAG3110_read(&Xcomp,&Ycomp,&Zcomp);
              if (x == averages-1) {
                Serial_Print("\"compass_x\":");
                Serial_Print_Float(Xcomp,6);
                Serial_Print(",");                
                Serial_Print("\"compass_y\":");
                Serial_Print_Float(Ycomp,6);
                Serial_Print(",");               
                Serial_Print("\"compass_z\":");
                Serial_Print_Float(Zcomp,6);
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "accel") {                      // perform analog reads
              int Xval, Yval, Zval;
              MMA8653FC_read(&Xval,&Yval,&Zval);
              if (x == averages-1) {
                Serial_Print("\"accel_x\":");
                Serial_Print_Float(Xval,6);
                Serial_Print(",");                
                Serial_Print("\"accel_y\":");
                Serial_Print_Float(Yval,6);
                Serial_Print(",");               
                Serial_Print("\"accel_z\":");
                Serial_Print_Float(Zval,6);
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "spec") {                      // perform analog reads
              readSpectrometer();
              spec_on = 1;                                                                       // throw flag to indicate that the spec is being used
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "digital_read") {                      // perform digital reads
              int pin = environmental.getArray(i).getLong(2);
              pinMode(pin,INPUT);
              int digital_read = digitalRead(pin);
              if (x == averages-1) {
                Serial_Print("\"digital_read\":");
                Serial_Print_Float(digital_read,2);  
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "digital_write") {                      // perform digital write
              int pin = environmental.getArray(i).getLong(2);
              int setting = environmental.getArray(i).getLong(3);
              pinMode(pin,OUTPUT);
              digitalWriteFast(pin,setting);
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "analog_write") {                      // perform analog write with length of time to apply the pwm
              int pin = environmental.getArray(i).getLong(2);
              int setting = environmental.getArray(i).getLong(3);
              int freq = environmental.getArray(i).getLong(4);
              int wait = environmental.getArray(i).getLong(5);
#ifdef DEBUGSIMPLE
              Serial_Print_Line(pin);
              Serial_Print_Line(pin);
              Serial_Print_Line(wait);
              Serial_Print_Line(setting);
              Serial_Print_Line(freq);              
#endif
              pinMode(pin,OUTPUT);
              analogWriteFrequency(pin, freq);                                                           // set analog frequency
              analogWrite(pin,setting);
              delay(wait);
              analogWrite(pin,0);
              reset_freq();                                                                              // reset analog frequencies
            }
            if (environmental.getArray(i).getLong(1) == 0 \
            && (String) environmental.getArray(i).getString(0) == "note") {                      // insert notes or other data.  Limit 999 chars, do not use following chars []+"'
              if (x == 0) {                                                                // if it's the last measurement to average, then print the results
                Serial_Print("\"note\":\"");
                String note = user_enter_str(3000000,0);                                            // wait for user to enter note
                Serial_Print(note.c_str());  
                Serial_Print("\",");                
              }
            }
          }

//&&
//          analogReadAveraging(analog_averages);                                      // set analog averaging (ie ADC takes one signal per ~3u)

//////////////////////ADC SETUP////////////////////////

adc->setAveraging(averaging0,ADC_0);                   // set number of averages
adc->setResolution(resolution0,ADC_0);                 // set bits of resolution

switch (conversion_speed) {    
  case 0:
    adc->setConversionSpeed(ADC_VERY_LOW_SPEED,ADC_0); // change the conversion speed
    break;
  case 1:
    adc->setConversionSpeed(ADC_LOW_SPEED,ADC_0); // change the conversion speed
    break;
  case 2:
    adc->setConversionSpeed(ADC_MED_SPEED,ADC_0); // change the conversion speed
    break;
  case 3:
    adc->setConversionSpeed(ADC_HIGH_SPEED_16BITS,ADC_0); // change the conversion speed
    break;
  case 4:
    adc->setConversionSpeed(ADC_HIGH_SPEED,ADC_0); // change the conversion speed
    break;
  case 5:
    adc->setConversionSpeed(ADC_VERY_HIGH_SPEED,ADC_0); // change the conversion speed
    break;
}

switch (sampling_speed) {    
  case 0:
    adc->setSamplingSpeed(ADC_VERY_LOW_SPEED,ADC_0); // change the sampling speed
    break;
  case 1:
    adc->setSamplingSpeed(ADC_LOW_SPEED,ADC_0); // change the sampling speed
    break;
  case 2:
    adc->setSamplingSpeed(ADC_MED_SPEED,ADC_0); // change the sampling speed
    break;
  case 3:
    adc->setSamplingSpeed(ADC_HIGH_SPEED_16BITS,ADC_0); // change the sampling speed
    break;
  case 4:
    adc->setSamplingSpeed(ADC_HIGH_SPEED,ADC_0); // change the sampling speed
    break;
  case 5:
    adc->setSamplingSpeed(ADC_VERY_HIGH_SPEED,ADC_0); // change the sampling speed
    break;
}

delay(2);

          int actfull = 0;
//          int _tcs_to_act = 0;
          float _light_intensity = lux_to_uE(lux_local);
//          _tcs_to_act = (uE_to_intensity(act_background_light,_light_intensity)*tcs_to_act)/100;  // set intensity of actinic background light
#ifdef DEBUGSIMPLE
          Serial_Print_Line("");
          Serial_Print("tcs to act: ");
          Serial_Print_Line(_tcs_to_act);
          Serial_Print("ambient light in uE: ");
          Serial_Print_Line(lux_to_uE(lux_local));
          Serial_Print("ue to intensity result:  ");
          Serial_Print_Line(uE_to_intensity(act_background_light,lux_to_uE(lux_local)));  // NOTE: wait turn flip DAC switch until later.
#endif
          for (int z=0;z<total_pulses;z++) {                                            // cycle through all of the pulses from all cycles
            int first_flag = 0;                                                           // flag to note the first pulse of a cycle
            int act1_on = 0;
            int act2_on = 0;
            int alt1_on = 0;
            int alt2_on = 0;
            int _spec = 0;                                                              // create the spec flag for the coralspeq
            if (cycle == 0 && pulse == 0) {                                             // if it's the beginning of a measurement, then...                                                             // wait a few milliseconds so that the actinic pulse presets can stabilize
//              volatile unsigned long* systimer = (volatile unsigned long*) 0xE000E018;                    // call system clock                          
              Serial.flush();                                                          // flush any remaining serial output info before moving forward          
              unsigned long starttimer0;
//              unsigned long timestart = 0;
//              unsigned long timeend = 0;
//              unsigned long systimerstart = 0;
//              unsigned long systimerend = 0;
//              unsigned short nowtime = 0;

//              while (*systimer > 1000) {                                                  // wait until timer is at the beginning of it's cycle
//              }

              digitalWriteFast(act_background_light_prev, LOW);                        // turn off actinic background light
//              starttimer0 = micros();
//              systimerstart = *systimer;



              timer0.begin(pulse1,pulsedistance);                                       // Begin firsts pulse
              noInterrupts();
              delayMicroseconds(pulsesize);
              interrupts();

              timer1.begin(pulse2,pulsedistance);                                       // Begin second pulse
            }      

#ifdef DEBUGSIMPLE
            Serial_Print_Line("");
            Serial_Print("cycle, measurement number, measurement array size,total pulses ");
            Serial_Print(cycle); 
            Serial_Print(", ");
            Serial_Print(meas_number);
            Serial_Print(", ");
            Serial_Print(meas_array_size);
            Serial_Print(",");
            Serial_Print_Line(total_pulses);
            Serial_Print("measurement light, detector,data raw average,data point        ");
            Serial_Print(_meas_light);
            Serial_Print(", ");
            Serial_Print_Line(detector);
#endif      
            if (pulse == 0) {                                                                             // if it's the first pulse
              meas_array_size = meas_lights.getArray(cycle).getLength();                                  // get the number of measurement/detector subsets in the new cycle
              first_flag = 1;                                                                             // flip flag indicating that it's the 0th pulse and a new cycle
            }

            _meas_light = meas_lights.getArray(cycle).getLong(meas_number%meas_array_size);               // move to next measurement light
            detector = detectors.getArray(cycle).getLong(meas_number%meas_array_size);                    // move to next detector

            if (pulse < meas_array_size) {                                                                // if it's the first pulse of a cycle, then change act 1 and 2, alt1 and alt2 values as per array's set at beginning of the file
              if (pulse == 0) {
                String _message_type = message.getArray(cycle).getString(0);                                // get what type of message it is
                if ((_message_type != "" || quit == -1) && x == 0) {                                         // if there are some messages or the user has entered -1 to quit AND it's the first repeat of an average (so it doesn't ask these question on every average), then print object name...
                  if (message_flag == 0) {                                                                 // if this is the first time the message has been printed, then print object name
                    Serial_Print("\"message\":[");
                    message_flag = 1;
                  }
//                  if (quit != -1) {                                                                          // if they haven't entered quit yet, check to make sure they didn't enter it since last time we checked
//                    quit = user_enter_long(5);                                                               // check to see if user has quit (send -1 on USB or bluetooth serial)
//                  }
//                  if (quit == -1) {                                                                          // if the user quit the measurement, then post any data produced so far (skipPart) and quit (skipall)
//                    Serial_Print("[]],");
//                    goto skipPart;
//                  }
                  Serial_Print("[\"");
                  Serial_Print(_message_type.c_str());                                                               // print message
                  Serial_Print("\",");
                  Serial_Print("\"");
                  Serial_Print(message.getArray(cycle).getString(1));
                  Serial_Print("\",");
                  if (_message_type == "0") {
                    Serial_Print("\"\"]");
                  }
                  else if (_message_type == "alert") {                                                    // wait for user response to alert
                    while (1) {
                      int response = user_enter_long(3000000);           
                      if (response == -1) {
                        Serial_Print("\"ok\"]");
                        break;
                      }
                    }
                  }
                  else if (_message_type == "confirm") {                                                  // wait for user's confirmation message.  If enters '1' then skip to end.
                    while (1) {
                      int response = user_enter_long(3000000);
                      if (response == 1) {
                        Serial_Print("\"cancel\"]],");
                        goto skipPart;
                      }
                      if (response == -1) {
                        Serial_Print("\"ok\"]");
                        break;
                      }
                    }
                  }
                  else if (_message_type == "prompt") {                                                    // wait for user to input information, followed by +
                    String response = user_enter_str(3000000,0);
                    Serial_Print("\"");
                    Serial_Print(response.c_str());
                    Serial_Print("\"]");
                  }             
                  if (cycle != pulses.getLength()-1) {                                                    // if it's not the last cycle, then add comma
                    Serial_Print(",");                  
                  }
                  else {                                                                                 // if it is the last cycle, then close out the array
                    Serial_Print("],");                  
                  }
                }

                _act1_light_prev = _act1_light;                                                           // save old actinic value as current value for act1,act2,alt1,and alt2
                _act1_light = act1_lights.getLong(cycle);
                act1_on = calculate_intensity(_act1_light,tcs_to_act,cycle,_light_intensity);             // calculate the intensities for each light and what light should be on or off.
                _act2_light_prev = _act2_light;
                _act2_light = act2_lights.getLong(cycle);
                act2_on = calculate_intensity(_act2_light,tcs_to_act,cycle,_light_intensity);
                _alt1_light_prev = _alt1_light;
                _alt1_light = alt1_lights.getLong(cycle);
                alt1_on = calculate_intensity(_alt1_light,tcs_to_act,cycle,_light_intensity);
                _alt2_light_prev = _alt2_light;
                _alt2_light = alt2_lights.getLong(cycle);
                alt2_on = calculate_intensity(_alt2_light,tcs_to_act,cycle,_light_intensity);
  
                _spec = spec.getLong(cycle);                                                      // pull whether the spec will get called in this cycle or not for coralspeq

              }
              calculate_intensity(_meas_light,tcs_to_act,cycle,_light_intensity);                      // in addition, calculate the intensity of the current measuring light

              switch (_meas_light) {                                                                    // set the DAC intensity for the measuring light only...
              case 15:
                dac.analogWrite(3,meas_intensity);
              case 16:
                dac.analogWrite(3,meas_intensity);
              case 11:
                dac.analogWrite(3,meas_intensity);
              case 12:
                dac.analogWrite(3,meas_intensity);
              case 20:
                dac.analogWrite(0,act_intensity); 
              case 2:
                dac.analogWrite(0,act_intensity);   
              case 10:
                dac.analogWrite(2,cal_intensity);        
              case 14:
                dac.analogWrite(2,cal_intensity);        
              }
              digitalWriteFast(DAC_ON, LOW);                                                       // and turn it on.
              delayMicroseconds(1);                                                     
              digitalWriteFast(DAC_ON, HIGH);                                              

              if (pulse == 0) {
                dac.analogWrite(0,act_intensity);                                                     // ...also write the new values to the DAC for actinic lights, but don't turn them on yet!     
                dac.analogWrite(3,meas_intensity);
                dac.analogWrite(2,cal_intensity);        
              }            

#ifdef DEBUGSIMPLE
              Serial_Print("actinic, measurement, and calibration intensities           ");
              Serial_Print(act_intensity);
              Serial_Print(",");
              Serial_Print(meas_intensity);
              Serial_Print(",");
              Serial_Print_Line(cal_intensity);

              Serial_Print_Line("state of actinic lights                                   ");         
              Serial_Print(act1_on);
              Serial_Print(",");
              Serial_Print(act2_on);
              Serial_Print(",");
              Serial_Print(alt1_on);
              Serial_Print(",");
              Serial_Print_Line(alt2_on);
#endif
            }

            if (user_enter_long(1) == -1) {
              q = number_of_protocols;
              y = measurements;
              u = protocols;
              x = averages;
              z = total_pulses;
            }

            while (on == 0 || off == 0) {                                                	 // if ALL pulses happened, then...
            }
//&&
//            data1 = analogRead(detector);                                                        // save the detector reading as data1    

/*

 **RUN(uint8_t mode, uint8_t woi) - 
 *      transition into BLPI mode which configures the core(2 MHZ), 
 *      bus(2 MHZ), flash(1 MHZ) clocks and configures SysTick for 
 *      the reduced freq, then enter into vlpr mode. Exiting Low Power 
 *      Run mode will transition to PEE mode and reconfigures clocks
 *      and SysTick to a pre RUN state and exit vlpr to normal run.
 * Parameter:  mode -> LP_RUN_ON = enter Low Power Run(VLPR)
 *             mode -> LP_RUN_OFF = exit Low Power Run(VLPR)
 *             woi  -> NO_WAKE_ON_INTERRUPT = no exit LPR on interrupt 
 *             woi  -> WAKE_ON_INTERRUPT = exit LPR on interrupt

*/

//            long started1 = micros();
//            LP.Run(LP_RUN_ON);
//            long ended1 = micros();

// normal way of doing it
            data1 = adc->analogRead(detector,ADC_0);


// performing medians

/*

delayMicroseconds(200);

            float data_array[50];
            long beginning = micros();
            for (int i = 0;i<50;i++) {
              data_array[i] = adc->analogRead(detector,ADC_0);
            }
//            long ending = micros();
            for (int i = 0;i<50;i++) {
              Serial_Print(data_array[i]);
              Serial_Print(",");
            }
//            long difference = ending - beginning;
//            Serial_Print(difference);
//            Serial_Print_Line("");
            Serial_Print_Line("");
            for (int i=0;i<50;i++) {
              for (int j=0;j<50;j++) {
                if (data_array[i] > data_array[j]) {
                  float temp = data_array[i];
                  data_array[i] = data_array[j];
                  data_array[j] = temp;
                }
              }
            }
//            for (int i=0;i<50;i++) {
//              Serial_Print(data_array[i]);
//              Serial_Print(",");
//            }   
            data1 = data_array[24];
//            Serial_Print_Line("");
//            Serial_Print("This is actual output  ");
//            Serial_Print_Line(data1);
            
*/            
            
            
/*            
            long started2 = micros();
            LP.Run(LP_RUN_OFF);
            long ended2 = micros();
*/
            digitalWriteFast(SAMPLE_AND_HOLD, HIGH);						 // turn off sample and hold, and turn on lights for next pulse set

            if (first_flag == 1) {                                                                    // if this is the 0th pulse and a therefore new cycle
              digitalWriteFast(_act1_light_prev, LOW);                                                // turn off previous lights, turn on the new ones on (if light setting is zero, then no light on
              if (act1_on == 1) {                                                                      // only turn on if your supposed to!
                digitalWriteFast(_act1_light, HIGH);
//                Serial_Print("act1 high!");
//                Serial_Print("act1 high!");
              }
              digitalWriteFast(_act2_light_prev, LOW);
              if (act2_on == 1) {
                digitalWriteFast(_act2_light, HIGH);
//                Serial_Print("act2 high!");
//                Serial_Print("act2 high!");
              }
              digitalWriteFast(_alt1_light_prev, LOW);
              if (alt1_on == 1) {
                digitalWriteFast(_alt1_light, HIGH);
              }
              digitalWriteFast(_alt2_light_prev, LOW);
              if (alt2_on == 1) {
                digitalWriteFast(_alt2_light, HIGH);
              } 

              digitalWriteFast(DAC_ON, LOW);                                               // now turn on the DAC with the new values for the next cycle
              delayMicroseconds(1);                                                     
              digitalWriteFast(DAC_ON, HIGH);                                              
              first_flag = 0;                                                              // reset flag
            }

            float offset = 0;
            if (offset_off == 0) {
              switch (detector) {                                                          // apply offset to whicever detector is being used
              case 34:
                offset = offset_34;
                break;
              case 35:
                offset = offset_35;
                break;
              }
            }

#ifdef DEBUGSIMPLE
            Serial_Print("data count, size of raw data                                   ");
            Serial_Print(data_count);  
            Serial_Print(",");
            Serial_Print_Line(size_of_data_raw);

#endif
            if (_spec != 1) {                                                    // if spec_on is not equal to 1, then coralspeq is off and proceed as per normal MultispeQ measurement.
              if (_meas_light  != 0) {                                                      // save the data, so long as the measurement light is not equal to zero.
                data_raw_average[data_count] += data1 - offset;  
                data_count++;
              }
            }
            else if (_spec == 1) {                                              // if spec_on is 1 for this cycle, then collect data from the coralspeq and save it to data_raw_average.
              readSpectrometer();                                                        // collect a reading from the spec
              for (int i=0 ; i < SPEC_CHANNELS; i++) {
                data_raw_average[data_count] += spec_data[i];
                data_count++;
              }
            } 
            noInterrupts();                                                              // turn off interrupts because we're checking volatile variables set in the interrupts
            on = 0;                                                                      // reset pulse counters
            off = 0;  
            pulse++;                                                                     // progress the pulse counter and measurement number counter

#ifdef DEBUGSIMPLE
            Serial_Print("data point average, current data                               ");
            Serial_Print(data_raw_average[meas_number]);
            Serial_Print("!");
            Serial_Print_Line(data1); 
#endif
            interrupts();                                                              // done with volatile variables, turn interrupts back on
            meas_number++;                                                              // progress measurement number counters

            if (pulse == pulses.getLong(cycle)*meas_lights.getArray(cycle).getLength()) { // if it's the last pulse of a cycle...
              pulse = 0;                                                               // reset pulse counter      
              cycle++;                                                                 // ...move to next cycle
            }
          }        
          background_on = 0;
          background_on = calculate_intensity_background(act_background_light,tcs_to_act,cycle,_light_intensity,act_background_light_intensity);  // figure out background light intensity and state

          if (_act1_light != act_background_light) {                                  // turn off all lights unless they are the actinic background light  
            digitalWriteFast(_act1_light, LOW);
          }
          if (_act2_light != act_background_light) {
            digitalWriteFast(_act2_light, LOW);
          }
          if (_alt1_light != act_background_light) {
            digitalWriteFast(_alt1_light, LOW);
          }
          if (_alt2_light != act_background_light) {
            digitalWriteFast(_alt2_light, LOW);
          }

          if (background_on == 1) {
            digitalWriteFast(DAC_ON, LOW);        
            delayMicroseconds(1);
            digitalWriteFast(DAC_ON, HIGH);
            digitalWriteFast(act_background_light, HIGH);                                // turn on actinic background light in case it was off previously.
          }
          else {
            digitalWriteFast(act_background_light, LOW);                                // turn on actinic background light in case it was off previously.
          }     

          timer0.end();                                                                  // if it's the last cycle and last pulse, then... stop the timers
          timer1.end();
          cycle = 0;                                                                     // ...and reset counters
          pulse = 0;
          on = 0;
          off = 0;
          meas_number = 0;

          /*
      options for relative humidity, temperature, contactless temperature. light_intensity,co2
           0 - take before spectroscopy measurements
           1 - take after spectroscopy measurements
           */

          for (int i=0;i<environmental.getLength();i++) {                                             // call environmental measurements after the spectroscopic measurement
#ifdef DEBUGSIMPLE
            Serial_Print_Line("Here's the environmental measurements called:    ");
            Serial_Print(environmental.getArray(i).getString(0));
            Serial_Print(", ");
            Serial_Print_Line(environmental.getArray(i).getLong(1));
#endif

            if (environmental.getArray(i).getLong(1) == 1 \                                       
            && (String) environmental.getArray(i).getString(0) == "relative_humidity") {
              Relative_Humidity((int) environmental.getArray(i).getLong(1));                        // if this string is in the JSON and the 3rd component in the array is == 1 (meaning they want this measurement taken prior to the spectroscopic measurement), then call the associated measurement (and so on for all if statements in this for loop)
              if (x == averages-1) {                                                                // if it's the last measurement to average, then print the results
                Serial_Print("\"relative_humidity\":");
                Serial_Print_Float(relative_humidity_average,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
          && (String) environmental.getArray(i).getString(0) == "temperature") {
              Temperature((int) environmental.getArray(i).getLong(1));
              if (x == averages-1) {
                Serial_Print("\"temperature\":");
                Serial_Print_Float(temperature_average,2);  
                Serial_Print(",");     
              }       
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "contactless_temperature") {
              Contactless_Temperature( environmental.getArray(i).getLong(1));
              float c_temp = MLX90614_Read(0);
              if (x == averages-1) {
                Serial_Print("\"contactless_temperature\":");
                Serial_Print_Float(c_temp,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
          && (String) environmental.getArray(i).getString(0) == "co2") {
              Co2( environmental.getArray(i).getLong(1));
              if (x == averages-1) {
                Serial_Print("\"co2\":");
                Serial_Print_Float(co2_value_average,2);  
                Serial_Print(",");
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
          && (String) environmental.getArray(i).getString(0) == "light_intensity") {
              Light_Intensity(1);
              if (x == averages-1) {
                Serial_Print("\"light_intensity\":");
                Serial_Print_Float(lux_to_uE(lux_average_forpar),2);  
                Serial_Print(",");                
                Serial_Print("\"r\":");
                Serial_Print_Float(lux_to_uE(r_average_forpar),2);  
                Serial_Print(",");  
                Serial_Print("\"g\":");
                Serial_Print_Float(lux_to_uE(g_average_forpar),2);  
                Serial_Print(",");  
                Serial_Print("\"b\":");
                Serial_Print_Float(lux_to_uE(b_average_forpar),2);  
                Serial_Print(",");  
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "light_intensity_raw") {
              Light_Intensity(0);
              if (x == averages-1) {
                Serial_Print("\"light_intensity_raw\":");
                Serial_Print_Float(lux_average,2);  
                Serial_Print(",");                
                Serial_Print("\"r\":");
                Serial_Print_Float(r_average,2);  
                Serial_Print(",");  
                Serial_Print("\"g\":");
                Serial_Print_Float(g_average,2);  
                Serial_Print(",");  
                Serial_Print("\"b\":");
                Serial_Print_Float(b_average,2);  
                Serial_Print(",");  
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "analog_read") {                      // perform analog reads
              int pin = environmental.getArray(i).getLong(2);
              pinMode(pin,INPUT);
              int analog_read = analogRead(pin);
              if (x == averages-1) {
                Serial_Print("\"analog_read\":");
                Serial_Print_Float(analog_read,4);  
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "compass") {                      // perform analog reads
              int Xcomp, Ycomp, Zcomp;
              MAG3110_read(&Xcomp,&Ycomp,&Zcomp);
              if (x == averages-1) {
                Serial_Print("\"compass_x\":");
                Serial_Print_Float(Xcomp,6);
                Serial_Print(",");                
                Serial_Print("\"compass_y\":");
                Serial_Print_Float(Ycomp,6);
                Serial_Print(",");               
                Serial_Print("\"compass_z\":");
                Serial_Print_Float(Zcomp,6);
                Serial_Print(",");             
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "accel") {                      // perform analog reads
              int Xval, Yval, Zval;
              MMA8653FC_read(&Xval,&Yval,&Zval);
              if (x == averages-1) {
                Serial_Print("\"accel_x\":");
                Serial_Print_Float(Xval,6);
                Serial_Print(",");                
                Serial_Print("\"accel_y\":");
                Serial_Print_Float(Yval,6);
                Serial_Print(",");               
                Serial_Print("\"accel_z\":");
                Serial_Print_Float(Zval,6);
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "digital_read") {                      // perform digital reads
              int pin = environmental.getArray(i).getLong(2);
              pinMode(pin,INPUT);
              int digital_read = digitalRead(pin);
              if (x == averages-1) {
                Serial_Print("\"digital_read\":");
                Serial_Print_Float(digital_read,0);  
                Serial_Print(",");                
              }
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "digital_write") {                      // perform digital write
              int pin = environmental.getArray(i).getLong(2);
              int setting = environmental.getArray(i).getLong(3);
              pinMode(pin,OUTPUT);
              digitalWriteFast(pin,setting);
            }            
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "analog_write") {                      // perform analog write with length of time to apply the pwm
              int pin = environmental.getArray(i).getLong(2);
              int setting = environmental.getArray(i).getLong(3);
              int freq = environmental.getArray(i).getLong(4);
              int wait = environmental.getArray(i).getLong(5);
#ifdef DEBUGSIMPLE
              Serial_Print_Line(pin);
              Serial_Print_Line(pin);
              Serial_Print_Line(wait);
              Serial_Print_Line(setting);
              Serial_Print_Line(freq);              
#endif
              pinMode(pin,OUTPUT);
              analogWriteFrequency(pin, freq);                                                           // set analog frequency
              analogWrite(pin,setting);
              delay(wait);
              analogWrite(pin,0);
              reset_freq();                                                                              // reset analog frequencies
            }
            if (environmental.getArray(i).getLong(1) == 1 \
            && (String) environmental.getArray(i).getString(0) == "note") {                      // insert notes or other data.  Limit 999 chars, do not use following chars []+"'
              if (x == 0) {                                                                // if it's the last measurement to average, then print the results
                Serial_Print("\"note\":\"");
                String note = user_enter_str(3000000,0);                                            // wait for user to enter note
                Serial_Print(note.c_str());  
                Serial_Print("\",");                
              }
            }            
          }
          if (x+1 < averages) {                                                               //  to next average, unless it's the end of the very last run
            if (averages_delay > 0) {
              user_enter_long(averages_delay*1000);
            }
            if (averages_delay_ms > 0) {
              user_enter_long(averages_delay_ms);
            }
          }
        }
        skipPart:                                                                             // skip to the end of the protocol

        if (spec_on == 1) {                                                                    // if the spec is being used, then read it and print data_raw as spec values.  Otherwise, print data_raw as multispeq detector values as per normal
          Serial_Print("\"data_raw\":[");
          for (int i = 0; i < SPEC_CHANNELS; i++) 
          {
            Serial_Print_Float(spec_data_average[i]/averages,2);
            if (i != SPEC_CHANNELS-1) {                                                       // if it's the last one in printed array, don't print comma
              Serial_Print(",");
            }
          }
          Serial_Print("]}");
        }
        else {
          Serial_Print("\"data_raw\":[");
          for (int i=0;i<size_of_data_raw;i++) {                                                  // print data_raw, divided by the number of averages
            Serial_Print_Float(data_raw_average[i]/averages,2);
            if (i != size_of_data_raw-1) {
              Serial_Print(",");
            }
          }
          Serial_Print("]}");
      }
#ifdef DEBUGSIMPLE
        Serial_Print("# of protocols repeats, current protocol repeat, number of total protocols, current protocol      ");
        Serial_Print(protocols);
        Serial_Print(",");
        Serial_Print(u);
        Serial_Print(",");
        Serial_Print(number_of_protocols);
        Serial_Print(",");
        Serial_Print_Line(q);
#endif 

        if (q < number_of_protocols-1 || u < protocols-1) {                               // if it's not the last protocol in the measurement and it's not the last repeat of the current protocol, add a comma
          Serial_Print(",");
          if (protocols_delay > 0) {
            user_enter_long(protocols_delay*1000);
          }
          if (protocols_delay_ms > 0) {
            user_enter_long(protocols_delay_ms);
          }
        }
/* 47 CHANGES */
//        else if (q == number_of_protocols-1 && u == protocols-1) {                      // if it is the last protocol, then close out the data json
//          Serial_Print("]");
//        }      

        averages = 1;   			                                        // number of times to repeat the entire run 
        averages_delay = 0;                  	                                        // seconds wait time between averages
        averages_delay_ms = 0;           	                                        // seconds wait time between averages
        analog_averages = 1;                                                             // # of measurements per pulse to be averaged (min 1 measurement per 6us pulselengthon)
        _act1_light = 0;
        _act2_light = 0;
        _alt1_light = 0;
        _alt2_light = 0;
        detector = 0;
        pulsesize = 0;                                                                // measured in microseconds
        pulsedistance = 0;
        act_intensity = 0;                                                            // intensity at LOW setting below
        meas_intensity = 0;                                                            // 255 is max intensity during pulses, 0 is minimum // for additional adjustment, change resistor values on the board
        cal_intensity = 0;
        relative_humidity_average = 0;                                                // reset all environmental variables to zero
        temperature_average = 0;
        objt_average = 0;
        co2_value_average = 0;
        lux_average = 0;
        r_average = 0;
        g_average = 0;
        b_average = 0;
        lux_average_forpar = 0;
        r_average_forpar = 0;
        g_average_forpar = 0;
        b_average_forpar = 0;
        for (int i = 0; i < SPEC_CHANNELS; i++) {
          spec_data_average [i] = 0;                 
        }        
        act_background_light_prev = act_background_light;                               // set current background as previous background for next protocol
        spec_on = 0;                                                                    // reset flag that spec is turned on for this measurement
#ifdef DEBUGSIMPLE
        Serial_Print_Line("previous light set to:   ");
        Serial_Print_Line(act_background_light_prev);
#endif    
      }
    }
    serial_bt_flush();

    if (y < measurements-1) {                                                      // add commas between measurements
      Serial_Print(",");
      if (measurements_delay > 0) {
        user_enter_long(measurements_delay*1000);
      }
      else if (measurements_delay_ms > 0) {
        user_enter_long(measurements_delay_ms);
      }
    }
  }
  skipall:
  Serial_Print("]}");
  digitalWriteFast(act_background_light, LOW);                                    // turn off the actinic background light at the end of all measurements
  act_background_light = 13;                                                      // reset background light to teensy pin 13
  free(data_raw_average);                                                         // free the calloc() of data_raw_average
  free(json);                                                                     // free second json malloc
  Serial_Print_CRC();
}

void pulse1() {		                                                        // interrupt service routine which turns the measuring light on
  digitalWriteFast(SAMPLE_AND_HOLD, LOW);		            		 // turn on measuring light and/or actinic lights etc., tick counter
  digitalWriteFast(_meas_light, HIGH);						// turn on measuring light
  on=1;
}

void pulse2() {    	                                                        // interrupt service routine which turns the measuring light off									// turn off measuring light, tick counter
  digitalWriteFast(_meas_light, LOW);
  off=1;
}



void lighttests_all() {

  // enter value to increment lights (suggested value is 40) followed by +
  // or press only +0 to exit

  double increment = user_enter_dbl(60000);

  Serial_Print("{\"response\":\"");

  if (increment == 0) {
    //back to main menu
    goto skipit;
  }
  Serial_Print_Float(increment,0);  
  Serial_Print(",");  
  Serial.read();
  Serial.read();

  digitalWriteFast(ACTINICLIGHT1,HIGH);
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print(0);
  Serial_Print("actinic intensity 1 (switch LOW)");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(0,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }

  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, LOW);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(1,0);
  Serial_Print("actinic intensity 1 (switch HIGH)");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(1,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(ACTINICLIGHT1,LOW);

  digitalWriteFast(ACTINICLIGHT2,HIGH);
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(0,0);
  Serial_Print("actinic intensity 2 (switch LOW)");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(0,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, LOW);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(1,0);
  Serial_Print("actinic intensity 2 (switch HIGH)");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(1,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(ACTINICLIGHT2,LOW);

  digitalWriteFast(CALIBRATINGLIGHT1,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(2,0);
  Serial_Print("calibrating light 1");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(2,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(CALIBRATINGLIGHT1,LOW);
  digitalWriteFast(CALIBRATINGLIGHT2,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(2,0);
  Serial_Print("calibrating light 2");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(2,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(CALIBRATINGLIGHT2,LOW);

  digitalWriteFast(MEASURINGLIGHT1,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(2,0);
  Serial_Print("measuring light 1");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(3,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(MEASURINGLIGHT1,LOW);
  digitalWriteFast(MEASURINGLIGHT2,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(1,0);
  Serial_Print("measuring light 2");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(3,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(MEASURINGLIGHT2,LOW);

  digitalWriteFast(MEASURINGLIGHT3,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(1,0);
  Serial_Print("measuring light 3");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(3,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(MEASURINGLIGHT3,LOW);
  digitalWriteFast(MEASURINGLIGHT4,HIGH);
  Serial_Print("DAC number:   ");
  Serial_Print_Float(1,0);
  Serial_Print("measuring light 4");
  for (int i=0;i<4095;i=i+increment) {
    dac.analogWrite(3,i); // write to input register of a DAC. Channel 0-3, Value 0-4095    
    Serial_Print_Float(i,0);
    Serial_Print(",");
    delay(50);
  }
  digitalWriteFast(MEASURINGLIGHT4,LOW);
skipit:
  serial_bt_flush();
  Serial_Print("\"}");
}

void lighttests(int _choose) {

  float sensor_value = 0;
  digitalWriteFast(DAC_ON, LOW);                                               // leave DAC on for light tests.

  dac.analogWrite(0,4095);                                                       // write to input register of a DAC. channel 0 for high (saturating).  0 (low) - 4095 (high).  1 step = +3.654uE  
  dac.analogWrite(1,4095);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, LOW);                         // preset the switch to the actinic (low) preset position
  dac.analogWrite(2,4095);                                                       // write to input register of a DAC. channel 2, calibrating light.  0 (low) - 4095 (high).  1 step = +3.654uE  
  dac.analogWrite(3,4095);                                                       // write to input register of a DAC. channel 3 measuring light.  0 (high) - 4095 (low).  2092 = 0.  From 2092 to zero, 1 step = +.2611uE
  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, HIGH);

  serial_bt_flush();
  if (_choose == 98) {
    call_print_calibration(0);
    int tcs_to_act = 100;                                                          // the actinic value is == to the ambient value (100 is ==, 200 is 2x, 50 is .5x, etc.)
    int _tcs_to_act = 0;                                                           // the actinic value is == to the ambient value (100 is ==, 200 is 2x, 50 is .5x, etc.)
    double dac_setting = 0;
    while (1) {
      dac_setting = user_enter_dbl(60000);
      if (dac_setting == -1) {
        goto end;
      }
      serial_bt_flush();
      dac.analogWrite(0,dac_setting);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      if (dac_setting != 0) {
        digitalWriteFast(ACTINICLIGHT1, HIGH);
      }
      else {
        digitalWriteFast(ACTINICLIGHT1, LOW);
      }
    }
end:
    serial_bt_flush();
    digitalWriteFast(ACTINICLIGHT1, LOW);
  }

  if (_choose == 99) {
    call_print_calibration(0);
    float ambient_light = 0;
    int _light = user_enter_dbl(60000);                                          // enter the light [15,16,11,2,20]
    int tcs_to_act = 100;                                                          // the actinic value is == to the ambient value (100 is ==, 200 is 2x, 50 is .5x, etc.)
    int _tcs_to_act = 0;                                                          // the actinic value is == to the ambient value (100 is ==, 200 is 2x, 50 is .5x, etc.)
    Serial_Print("{\"act_to_tcs\":[");
    dac.analogWrite(0,0);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
    dac.analogWrite(3,0);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
    while (Serial.available()<3 && Serial1.available()<3) {
      digitalWriteFast(_light, HIGH);
      sensor_value = lux_to_uE(Light_Intensity(0));
      _tcs_to_act = (uE_to_intensity(_light,sensor_value)*tcs_to_act)/100;
#ifdef DEBUGSIMPLE
      Serial_Print(sensor_value);
      Serial_Print(",");
      Serial_Print(lux_to_uE(sensor_value));
      Serial_Print(",");
      Serial_Print(tcs_to_act);
      Serial_Print("");
#endif
      if (sensor_value == 0) {                                                              // if the ambient light is zero, then turn off the light completely
        digitalWriteFast(_light, LOW);
      }
      else {
        dac.analogWrite(0,_tcs_to_act);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
        dac.analogWrite(3,_tcs_to_act);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      }
      Serial_Print_Float(sensor_value,4);
      Serial_Print(",");
      Serial_Print_Float(_tcs_to_act,2);
      Serial_Print(",");
      delay(1000);
    }
    serial_bt_flush();
    digitalWriteFast(_light, LOW);      
  }
  serial_bt_flush(); 
  if (_choose == 100) {
    lighttests_all();
  }
  serial_bt_flush(); 
  if (_choose == 101) {
    Serial_Print("{\"light_intensity\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = lux_to_uE(Light_Intensity(1));
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(2000);
    }
    serial_bt_flush();
  }
  else if (_choose == 102) {                      // co2
    Serial_Print("{\"Co2_value\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = Co2(1);
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(2000);
    }
    serial_bt_flush();
  }
  else if (_choose == 103) {                      // temperature
    Serial_Print("{\"temperature\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = Temperature(1);
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(2000);
    }
    serial_bt_flush();
  }
  else if (_choose == 104) {                      //relative humidity
    Serial_Print("{\"relative_humidity\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = Relative_Humidity(1);
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(2000);
    }
    serial_bt_flush();
  }
  else if (_choose == 105) {
    Serial_Print("{\"light_intensity_raw\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = Light_Intensity(0);
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(2000);
    }
    serial_bt_flush();
  }
  else if (_choose == 106) {
    Serial_Print("{\"contactless_temperature\":[");
    while (Serial.available()<3 && Serial1.available()<3) {
      sensor_value = Contactless_Temperature(1);
      Serial_Print_Float(sensor_value,2);
      Serial_Print(",");
      delay(200);
    }
    serial_bt_flush();
  }

  if (_choose<34 && _choose>0) {
    Serial_Print("{[");
    int dac_setting = 0;
    while (1) {
      dac_setting = user_enter_dbl(60000);
      if (dac_setting == -1) {
        goto last;
      }
      serial_bt_flush();
      dac.analogWrite(0,dac_setting);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      dac.analogWrite(1,dac_setting);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      dac.analogWrite(2,dac_setting);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      dac.analogWrite(3,dac_setting);                                                       // write to input register of a DAC. channel 1, for low (actinic).  0 (low) - 4095 (high).  1 step = +3.69uE
      digitalWriteFast(_choose, HIGH);      
      if (dac_setting != 0) {
        digitalWriteFast(_choose, HIGH);      
      }
      else {
        digitalWriteFast(_choose, LOW);      
      }
    }
last:
    serial_bt_flush();
    digitalWriteFast(_choose, LOW);      
  }
  else if (_choose>33 && _choose <38) {
    int reading = 0;
    int pulsedistance = 0;
    while (1) {
      pulsedistance = user_enter_dbl(60000);
      if (pulsedistance == -1) {
        goto final;
      }
      digitalWriteFast(SAMPLE_AND_HOLD, LOW);
      delayMicroseconds(pulsedistance);
      reading = analogRead(_choose);
      digitalWriteFast(SAMPLE_AND_HOLD, HIGH);
      Serial_Print_Float(reading,2);
      Serial_Print(",");
    }
final:
    serial_bt_flush();
  }
  digitalWriteFast(DAC_ON, HIGH);                                               // turn DAC back off
  Serial_Print_Float(sensor_value,2);
  Serial_Print("]}");
}

float Relative_Humidity(int var1) {
  if (var1 == 1 || var1 == 0) {
    float relative_humidity = htu.readHumidity();
#ifdef DEBUGSIMPLE
    Serial_Print("\"relative_humidity\":");
    Serial_Print(relative_humidity);  
    Serial_Print(",");
#endif
    relative_humidity_average += relative_humidity / averages;
    return relative_humidity;
  }
}

float Temperature(int var1) {
  if (var1 == 1 || var1 == 0) {
    float temperature = htu.readTemperature();
#ifdef DEBUGSIMPLE
    Serial_Print("\"temperature\":");
    Serial_Print(temperature);  
    Serial_Print(",");
#endif
    temperature_average += temperature / averages;
    return temperature;
  }
}

int Light_Intensity(int var1) {
    lux_local = TCS3471.readCData();                  // take 3 measurements, outputs in format - = 65535 or whatever 16 bits is.
    r_local = TCS3471.readRData();
    g_local = TCS3471.readGData();
    b_local = TCS3471.readBData();
#ifdef DEBUGSIMPLE
    Serial_Print("\"light_intensity\": ");
    Serial_Print(lux_local, DEC);
    Serial_Print(",");
    Serial_Print("\"red\": ");
    Serial_Print(r_local, DEC);
    Serial_Print(",");
    Serial_Print("\"green\": ");
    Serial_Print(g_local, DEC);
    Serial_Print(",");
    Serial_Print("\"blue\": ");
    Serial_Print(b_local, DEC);
    Serial_Print(",");
    //  Serial_Print("\cyan\": ");
    //  Serial_Print(c, DEC);
#endif
    if (var1 == 0) {
      lux_average += lux_local / averages;
      r_average += r_local / averages;
      g_average += g_local / averages;
      b_average += b_local / averages;
}
    if (var1 == 1) {
      lux_average_forpar += lux_local / averages;
      r_average_forpar += r_local / averages;
      g_average_forpar += g_local / averages;
      b_average_forpar += b_local / averages;
}
  return lux_local;
}

void print_sensor_calibration(int _open) {
  if (_open == 0) {
    Serial_Print("{");
  }    
  Serial_Print("\"light_slope\":");
  Serial_Print_Float(light_slope,6);
  Serial_Print(",");
  Serial_Print("\"light_y_intercept\":");
  Serial_Print_Float(light_y_intercept,6);
  if (_open == 1) {
    Serial_Print(",");
  }    
  else {
    Serial_Print("}");
  }
}

String user_enter_str (long timeout,int _pwr_off) {
  Serial.setTimeout(timeout);
  Serial1.setTimeout(timeout);
  char serial_buffer [1000] = {0};
  String serial_string;
  //  serial_bt_flush();
  long start1 = millis();
  long end1 = millis();
#ifdef DEBUG
  Serial_Print_Line("");
  Serial_Print(Serial.available());
  Serial_Print(",");
  Serial_Print_Line(Serial1.available());
#endif  
  while (Serial.available() == 0 && Serial1.available() == 0) {
    if (_pwr_off == 1) {
      end1 = millis();
//      delay(100);
//      Serial_Print_Line(end1 - start1);
      if ((end1 - start1) > pwr_off_ms[0]) {
        pwr_off();
        goto skip;
      }
    }
  }
skip:
  if (Serial.available()>0) {                                                          // if it's from USB, then read it.
    if (Serial.peek() != '[') {
      Serial.readBytesUntil('+',serial_buffer, 1000);
      serial_string = serial_buffer;
      //        serial_bt_flush();
    }
    else {
    }
  }
  else if (Serial1.available()>0) {                                                    // if it's from bluetooth, then read it instead.
    if (Serial1.peek() != '[') {
      Serial1.readBytesUntil('+',serial_buffer, 1000);
      serial_string = serial_buffer;
      //        serial_bt_flush();
    }
    else {
    }
  }
  Serial.setTimeout(1000);
  Serial1.setTimeout(1000);
  return serial_string;
}

long user_enter_long(long timeout) {
  Serial.setTimeout(timeout);
  Serial1.setTimeout(timeout);
  long val;  
  char serial_buffer [32] = {
    0  };
  String serial_string;
  long start1 = millis();
  long end1 = millis();
  while (Serial.available() == 0 && Serial1.available() == 0) {
    end1 = millis();
    if ((end1 - start1) > timeout) {
      goto skip;
    }
  }                        // wait until some info comes in over USB or bluetooth...
skip:
  if (Serial1.available()>0) {                                                    // if it's from bluetooth, then read it instead.
    Serial1.readBytesUntil('+',serial_buffer, 32);
    serial_string = serial_buffer;
    val = atol(serial_string.c_str());  
  }
  else if (Serial.available()>0) {                                                          // if it's from USB, then read it.
    Serial.readBytesUntil('+',serial_buffer, 32);
    serial_string = serial_buffer;
    val = atol(serial_string.c_str());  
  }
  return val;
  while (Serial.available()>0) {                                                     // flush any remaining serial data before returning
    Serial.read();
  }
  while (Serial1.available()>0) {
    Serial1.read();
  }
  Serial.setTimeout(1000);
  Serial1.setTimeout(1000);
}

double user_enter_dbl(long timeout) {
  Serial.setTimeout(timeout);
  Serial1.setTimeout(timeout);
  double val;
  char serial_buffer [32] = {
    0  };
  String serial_string;
  long start1 = millis();
  long end1 = millis();
  while (Serial.available() == 0 && Serial1.available() == 0) {
    end1 = millis();
    if ((end1 - start1) > timeout) {
      goto skip;
    }
  }                        // wait until some info comes in over USB or bluetooth...
skip:
  if (Serial.available()>0) {                                                          // if it's from USB, then read it.
    Serial.readBytesUntil('+',serial_buffer, 32);
    serial_string = serial_buffer;
    val = atof(serial_string.c_str());  
  }
  else if (Serial1.available()>0) {                                                    // if it's from bluetooth, then read it instead.
    Serial1.readBytesUntil('+',serial_buffer, 32);
    serial_string = serial_buffer;
    val = atof(serial_string.c_str());  
  }
  return val;
  while (Serial.available()>0) {                                                     // flush any remaining serial data before returning
    Serial.read();
  }
  while (Serial1.available()>0) {
    Serial1.read();
  }
  Serial.setTimeout(1000);
  Serial1.setTimeout(1000);
}

float lux_to_uE(float _lux_average) {                                                      // convert the raw signal value to uE, based on a calibration curve
  float uE = (_lux_average-light_y_intercept)/light_slope;           
#ifdef DEBUGSIMPLE
  Serial_Print(_lux_average);
  Serial_Print(",");
  Serial_Print(light_y_intercept);
  Serial_Print(",");
  Serial_Print(light_slope);
  Serial_Print(",");
  Serial_Print_Line(uE);
#endif
  return uE;
}

int uE_to_intensity(int _pin, int _uE) {                                                  // convert PAR value from ambient uE to LED intensity, based on a specific chosen LED
  float _slope = 0;
  float _yint = 0;
  float intensity_drift_slope = 0;
  float intensity_drift_yint = 0;
  unsigned int _intensity = 0;
  for (int i=0;i<sizeof(all_pins)/sizeof(int);i++) {                                                      // loop through all_pins
    if (all_pins[i] == _pin) {                                                                        // when you find the pin your looking for
      intensity_drift_slope = (calibration_slope_factory[i] - calibration_slope[i]) / calibration_slope_factory[i];
      intensity_drift_yint = (calibration_yint_factory[i] - calibration_yint[i]) / calibration_yint_factory[i];
      _slope = calibration_other1[i]+calibration_other1[i]*intensity_drift_slope;                                                                  // go get the calibration slope and yintercept, multiply by the intensity drift
      _yint = calibration_other2[i]+calibration_other2[i]*intensity_drift_yint;
      break;
    }
  }
  if (_slope != 0 || _yint != 0) {                                                                      // if calibration values exist then...
    _intensity = (_uE-_yint)/_slope;                                                                    // calculate the resulting intensity DAC value
  }
#ifdef DEBUGSIMPLE
  Serial_Print("uE, slope, yint, act_background pin, DAC intensity:   ");
  Serial_Print(_uE);
  Serial_Print(",");
  Serial_Print(_slope);
  Serial_Print(",");
  Serial_Print(_yint);
  Serial_Print(",");
  Serial_Print_Line(_intensity);
#endif
  return _intensity;
}

//350 - 1150 - light calibrations
//1150 - 1550 - baseline
//1550 - 1750 - spad blanks
//1750 - 2150 - other additional user defined calibrations

void print_cal_userdef(String name, float array[],int last,int array_length) {                                                    // little function to clean up printing calibration values

  Serial_Print("\"");
  Serial_Print(name.c_str());
  Serial_Print("\":[");
  for (int i=0;i<array_length;i++) {                                                      // recall the calibration arrays
    Serial_Print_Float(array[i],0);
    if (i != array_length-1) {        
      Serial_Print(",");    
    }
  }
  Serial_Print("]");    
  if (last != 1) {                                                                                        // if it's not the last one, then add comma.  otherwise, add curly.
    Serial_Print(",");    
  }
  else { 
    Serial_Print("}");
  }
}

void print_cal(String name, float array[],int last) {                                                    // little function to clean up printing calibration values
  Serial_Print("\"");
  Serial_Print(name.c_str());
  Serial_Print("\":[");
  for (int i=0;i<sizeof(all_pins)/sizeof(int);i++) {                                                      // recall the calibration arrays
    Serial_Print_Float(array[i],0);
    if (i != sizeof(all_pins)/sizeof(int)-1) {        
      Serial_Print(",");    
    }
  }
  Serial_Print("]");    
  if (last != 1) {                                                                                        // if it's not the last one, then add comma.  otherwise, add curly.
    Serial_Print(",");    
  }
  else { 
    Serial_Print("}");
  }
}

void reset_all(int which) {
  int clean_part1 [1439] = {};
  int clean_part2 [320] = {};
  if (which == 0) {
    EEPROM_writeAnything(0,clean_part1);                                                      // only reset the arrays, leave other calibrations
    EEPROM_writeAnything(1448,clean_part2);                                                      // only reset the arrays, leave other calibrations
  }
  else {
    EEPROM_writeAnything(60,clean_part1);                                                      // only reset the arrays, leave other calibrations
    EEPROM_writeAnything(1448,clean_part2);                                                      // only reset the arrays, leave other calibrations
  }
  Serial_Print("{\"complete\":\"True\"}");
  Serial_Print_CRC();
  call_print_calibration(1);
}

void call_print_calibration (int _print) {
  
  EEPROM_readAnything(0,tmp006_cal_S);
  EEPROM_readAnything(4,light_slope);
  EEPROM_readAnything(8,light_y_intercept);
  EEPROM_readAnything(12,device_id);
  EEPROM_readAnything(16,device_version);
  EEPROM_readAnything(20,manufacture_date);
  EEPROM_readAnything(24,slope_34);
  EEPROM_readAnything(28,yintercept_34);
  EEPROM_readAnything(32,slope_35);
  EEPROM_readAnything(36,yintercept_35);
  EEPROM_readAnything(40,userdef0);
  EEPROM_readAnything(60,calibration_slope);
  EEPROM_readAnything(180,calibration_yint);
  EEPROM_readAnything(300,calibration_slope_factory);
  EEPROM_readAnything(420,calibration_yint_factory);
  EEPROM_readAnything(540,calibration_baseline_slope);
  EEPROM_readAnything(660,calibration_baseline_yint);
  EEPROM_readAnything(880,calibration_blank1);
  EEPROM_readAnything(1000,calibration_blank2);
  EEPROM_readAnything(1120,calibration_other1);
  EEPROM_readAnything(1240,calibration_other2);
  EEPROM_readAnything(1360,userdef1);
  EEPROM_readAnything(1368,userdef2);
  EEPROM_readAnything(1376,userdef3);
  EEPROM_readAnything(1384,userdef4);
  EEPROM_readAnything(1392,userdef5);
  EEPROM_readAnything(1400,userdef6);
  EEPROM_readAnything(1408,userdef7);
  EEPROM_readAnything(1416,userdef8);
  EEPROM_readAnything(1424,userdef9);
  EEPROM_readAnything(1432,userdef10);
  EEPROM_readAnything(1440,pwr_off_ms);
  EEPROM_readAnything(1448,userdef11);
  EEPROM_readAnything(1456,userdef12);
  EEPROM_readAnything(1464,userdef13);
  EEPROM_readAnything(1760,userdef14);
  EEPROM_readAnything(1472,userdef15);
  EEPROM_readAnything(1480,userdef16);
  EEPROM_readAnything(1488,userdef17);
  EEPROM_readAnything(1496,userdef18);
  EEPROM_readAnything(1504,userdef19);
  EEPROM_readAnything(1512,userdef20);
  EEPROM_readAnything(1520,userdef21);
  EEPROM_readAnything(1528,userdef22);
  EEPROM_readAnything(1536,userdef23);
  EEPROM_readAnything(1544,userdef24);
  EEPROM_readAnything(1552,userdef25);
  EEPROM_readAnything(1560,userdef26);
  EEPROM_readAnything(1568,userdef27);
  EEPROM_readAnything(1576,userdef28);
  EEPROM_readAnything(1584,userdef29);
  EEPROM_readAnything(1592,userdef30);
  EEPROM_readAnything(1600,userdef31);
  EEPROM_readAnything(1608,userdef32);
  EEPROM_readAnything(1616,userdef33);
  EEPROM_readAnything(1624,userdef34);
  EEPROM_readAnything(1632,userdef35);
  EEPROM_readAnything(1640,userdef36);
  EEPROM_readAnything(1648,userdef37);
  EEPROM_readAnything(1656,userdef38);
  EEPROM_readAnything(1664,userdef39);
  EEPROM_readAnything(1672,userdef40);
  EEPROM_readAnything(1680,userdef41);
  EEPROM_readAnything(1688,userdef42);
  EEPROM_readAnything(1696,userdef43);
  EEPROM_readAnything(1704,userdef44);
  EEPROM_readAnything(1712,userdef45);
  EEPROM_readAnything(1720,userdef46);
  EEPROM_readAnything(1732,userdef47);
  EEPROM_readAnything(1744,userdef48);
  EEPROM_readAnything(1756,userdef49);
  EEPROM_readAnything(1768,userdef50);
  // 1780 - 1792 has DEVICE_FIRMWARE SAVED - do not use!

  if (_print == 1) {                                                                                      // if this should be printed to COM port --
    Serial_Print("{");
    print_offset(1);
    print_sensor_calibration(1);
    print_cal("all_pins", all_pins,0);
    print_cal("calibration_slope", calibration_slope,0);
    print_cal("calibration_yint", calibration_yint ,0);
    print_cal("calibration_slope_factory", calibration_slope_factory ,0);
    print_cal("calibration_yint_factory", calibration_yint_factory ,0);
    print_cal("calibration_baseline_slope", calibration_baseline_slope ,0);
    print_cal("calibration_baseline_yint", calibration_baseline_yint ,0);
    print_cal("calibration_blank1", calibration_blank1 ,0);
    print_cal("calibration_blank2", calibration_blank2 ,0);
    print_cal("calibration_other1", calibration_other1 ,0);
    print_cal("calibration_other2", calibration_other2 ,0);
    print_cal_userdef("userdef0", userdef0 ,0,2);
    print_cal_userdef("userdef1", userdef1 ,0,2);
    print_cal_userdef("userdef2", userdef2 ,0,2);
    print_cal_userdef("userdef3", userdef3 ,0,2);
    print_cal_userdef("userdef4", userdef4 ,0,2);
    print_cal_userdef("userdef5", userdef5 ,0,2);
    print_cal_userdef("userdef6", userdef6 ,0,2);
    print_cal_userdef("userdef7", userdef7 ,0,2);
    print_cal_userdef("userdef8", userdef8 ,0,2);
    print_cal_userdef("userdef9", userdef9 ,0,2);
    print_cal_userdef("userdef10", userdef10 ,0,2);
    print_cal_userdef("userdef11", userdef11 ,0,2);
    print_cal_userdef("userdef12", userdef12 ,0,2);
    print_cal_userdef("userdef13", userdef13 ,0,2);
    print_cal_userdef("userdef14", userdef14 ,0,2);
    print_cal_userdef("userdef15", userdef15 ,0,2);
    print_cal_userdef("userdef16", userdef16 ,0,2);
    print_cal_userdef("userdef17", userdef17 ,0,2);
    print_cal_userdef("userdef18", userdef18 ,0,2);
    print_cal_userdef("userdef19", userdef19 ,0,2);
    print_cal_userdef("userdef20", userdef20 ,0,2);
    print_cal_userdef("userdef21", userdef21 ,0,2);
    print_cal_userdef("userdef22", userdef22 ,0,2);
    print_cal_userdef("userdef23", userdef23 ,0,2);
    print_cal_userdef("userdef24", userdef24 ,0,2);
    print_cal_userdef("userdef25", userdef25 ,0,2);
    print_cal_userdef("userdef26", userdef26 ,0,2);
    print_cal_userdef("userdef27", userdef27 ,0,2);
    print_cal_userdef("userdef28", userdef28 ,0,2);
    print_cal_userdef("userdef29", userdef29 ,0,2);
    print_cal_userdef("userdef30", userdef30 ,0,2);
    print_cal_userdef("userdef31", userdef31 ,0,2);
    print_cal_userdef("userdef32", userdef32 ,0,2);
    print_cal_userdef("userdef33", userdef33 ,0,2);
    print_cal_userdef("userdef34", userdef34 ,0,2);
    print_cal_userdef("userdef35", userdef35 ,0,2);
    print_cal_userdef("userdef36", userdef36 ,0,2);
    print_cal_userdef("userdef37", userdef37 ,0,2);
    print_cal_userdef("userdef38", userdef38 ,0,2);
    print_cal_userdef("userdef39", userdef39 ,0,2);
    print_cal_userdef("userdef40", userdef40 ,0,2);
    print_cal_userdef("userdef41", userdef41 ,0,2);
    print_cal_userdef("userdef42", userdef42 ,0,2);
    print_cal_userdef("userdef43", userdef43 ,0,2);
    print_cal_userdef("userdef44", userdef44 ,0,2);
    print_cal_userdef("userdef45", userdef45 ,0,2);
    print_cal_userdef("userdef46", userdef46 ,0,3);
    print_cal_userdef("userdef47", userdef47 ,0,3);
    print_cal_userdef("userdef48", userdef48 ,0,3);
    print_cal_userdef("userdef49", userdef49 ,0,3);
    print_cal_userdef("userdef50", userdef50 ,0,3);
    print_cal_userdef("pwr_off_ms", pwr_off_ms,1,2);
    Serial_Print_CRC();
  }
}

void save_calibration_slope (int _pin,float _slope_val,int location) {
  for (int i=0;i<sizeof(all_pins)/sizeof(int);i++) {                                                      // loop through
    if (all_pins[i] == _pin) {                                                                        // when you find the pin your looking for
      EEPROM_writeAnything(location+i*4,_slope_val);
      //        save_eeprom_dbl(_slope_val,location+i*10);                                                                   // then in the same index location in the calibrations array, save the inputted value.
    }
  }
}

float save_calibration_yint (int _pin,float _yint_val,int location) {
  for (int i=0;i<sizeof(all_pins)/sizeof(int);i++) {                                                      // loop through all_pins
    if (all_pins[i] == _pin) {                                                                        // when you find the pin your looking for
      EEPROM_writeAnything(location+i*4,_yint_val);
      //        save_eeprom_dbl(_yint_val,location+i*10);                                                                   // then in the same index location in the calibrations array, save the inputted value.
    }
  }
}

void add_calibration (int location) {                                                                 // here you can save one of the calibration values.  This may be a regular calibration, or factory calibration (which saves both factory and regular values)
  call_print_calibration(1);                                                                            // call and print calibration info from eeprom
  int pin = 0;
  float slope_val = 0;
  float yint_val = 0;
  while (1) {
    pin = user_enter_dbl(60000);                                                                      // define the pin to add a calibration value to
    if (pin == -1) {                                                                                    // if user enters -1, exit from this calibration
      goto final;
    }
    slope_val = user_enter_dbl(60000);                                                                  // now enter that calibration value
    if (slope_val == -1) {
      goto final;
    }
    yint_val = user_enter_dbl(60000);                                                                  // now enter that calibration value
    if (yint_val == -1) {
      goto final;
    }                                                                                            // THIS IS STUPID... not sure why but for some reason you have to split up the eeprom saves or they just won't work... at leats this works...
    save_calibration_slope(pin,slope_val,location);                                                      // save the value in the calibration string which corresponding pin index in all_pins
    save_calibration_yint(pin,yint_val,location+120);                                                      // save the value in the calibration string which corresponding pin index in all_pins
skipit:
    delay(1);
  }
final:

  call_print_calibration(1);
}

void calculate_offset(int _pulsesize) {                                                                    // calculate the offset, based on the pulsesize and the calibration values offset = a*'pulsesize'+b
  offset_34 = slope_34*_pulsesize+yintercept_34;
  offset_35 = slope_35*_pulsesize+yintercept_35;
#ifdef DEBUGSIMPLE
  Serial_Print("offset for detector 34, 35   ");
  Serial_Print(offset_34,2);
  Serial_Print(",");
  Serial_Print(offset_35,2);
#endif
}

void print_offset(int _open) {
  if (_open == 0) {
    Serial_Print("{");  
  }
  Serial_Print("\"slope_34\":");
  Serial_Print_Float(slope_34,6);
  Serial_Print(",");
  Serial_Print("\"yintercept_34\":");
  Serial_Print_Float(yintercept_34,6);
  Serial_Print(",");
  Serial_Print("\"slope_35\":");
  Serial_Print_Float(slope_35,6);
  Serial_Print(",");
  Serial_Print("\"yintercept_35\":");
  Serial_Print_Float(yintercept_35,6);

  if (_open == 1) {
    Serial_Print(",");
  }
  else {
    Serial_Print("}");
    Serial_Print_CRC();
  }
}

void print_get_userdef0(int _0,int _1,int _2,int _3,int _4,int _5,int _6,int _7,int _8,int _9,int _10,int _11,int _12,int _13,int _14,int _15,int _16,int _17,int _18,int _19,int _20,int _21,int _22,int _23,int _24,int _25,int _26,int _27,int _28,int _29,int _30,int _31,int _32,int _33,int _34,int _35,int _36,int _37,int _38,int _39,int _40,int _41,int _42,int _43,int _44,int _45,int _46,int _47,int _48,int _49,int _50) {
    if (_0 == 1) {
      Serial_Print("\"get_userdef0\":[");
      Serial_Print_Float(userdef0[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef0[1],6);
      Serial_Print("],");
    }      
    if (_1 == 1) {
      Serial_Print("\"get_userdef1\":[");
      Serial_Print_Float(userdef1[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef1[1],6);
      Serial_Print("],");
    }      
    if (_2 == 1) {
      Serial_Print("\"get_userdef2\":[");
      Serial_Print_Float(userdef2[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef2[1],6);
      Serial_Print("],");
    }      
    if (_3 == 1) {
      Serial_Print("\"get_userdef3\":[");
      Serial_Print_Float(userdef3[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef3[1],6);
      Serial_Print("],");
    }      
    if (_4 == 1) {
      Serial_Print("\"get_userdef4\":[");
      Serial_Print_Float(userdef4[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef4[1],6);
      Serial_Print("],");
    }      
    if (_5 == 1) {
      Serial_Print("\"get_userdef5\":[");
      Serial_Print_Float(userdef5[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef5[1],6);
      Serial_Print("],");
    }      
    if (_6 == 1) {
      Serial_Print("\"get_userdef6\":[");
      Serial_Print_Float(userdef6[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef6[1],6);
      Serial_Print("],");
    }      
    if (_7 == 1) {
      Serial_Print("\"get_userdef7\":[");
      Serial_Print_Float(userdef7[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef7[1],6);
      Serial_Print("],");
    }      
    if (_8 == 1) {
      Serial_Print("\"get_userdef8\":[");
      Serial_Print_Float(userdef8[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef8[1],6);
      Serial_Print("],");
    }      
    if (_9 == 1) {
      Serial_Print("\"get_userdef9\":[");
      Serial_Print_Float(userdef9[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef9[1],6);
      Serial_Print("],");
    }      
    if (_10 == 1) {
      Serial_Print("\"get_userdef10\":[");
      Serial_Print_Float(userdef10[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef10[1],6);
      Serial_Print("],");
    }      
    if (_11 == 1) {
      Serial_Print("\"get_userdef11\":[");
      Serial_Print_Float(userdef11[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef11[1],6);
      Serial_Print("],");
    }      
    if (_12 == 1) {
      Serial_Print("\"get_userdef12\":[");
      Serial_Print_Float(userdef12[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef12[1],6);
      Serial_Print("],");
    }      
    if (_13 == 1) {
      Serial_Print("\"get_userdef13\":[");
      Serial_Print_Float(userdef13[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef13[1],6);
      Serial_Print("],");
    }      
    if (_14 == 1) {
      Serial_Print("\"get_userdef14\":[");
      Serial_Print_Float(userdef14[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef14[1],6);
      Serial_Print("],");
    }      
    if (_15 == 1) {
      Serial_Print("\"get_userdef15\":[");
      Serial_Print_Float(userdef15[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef15[1],6);
      Serial_Print("],");
    } 
    if (_16 == 1) {
      Serial_Print("\"get_userdef16\":[");
      Serial_Print_Float(userdef16[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef16[1],6);
      Serial_Print("],");
    }      
    if (_17 == 1) {
      Serial_Print("\"get_userdef17\":[");
      Serial_Print_Float(userdef17[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef17[1],6);
      Serial_Print("],");
    }      
    if (_18 == 1) {
      Serial_Print("\"get_userdef18\":[");
      Serial_Print_Float(userdef18[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef18[1],6);
      Serial_Print("],");
    }      
    if (_19 == 1) {
      Serial_Print("\"get_userdef19\":[");
      Serial_Print_Float(userdef19[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef19[1],6);
      Serial_Print("],");
    }      
    if (_20 == 1) {
      Serial_Print("\"get_userdef20\":[");
      Serial_Print_Float(userdef20[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef20[1],6);
      Serial_Print("],");
    }      
    if (_21 == 1) {
      Serial_Print("\"get_userdef21\":[");
      Serial_Print_Float(userdef21[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef21[1],6);
      Serial_Print("],");
    }      
    if (_22 == 1) {
      Serial_Print("\"get_userdef22\":[");
      Serial_Print_Float(userdef22[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef22[1],6);
      Serial_Print("],");
    }      
    if (_23 == 1) {
      Serial_Print("\"get_userdef23\":[");
      Serial_Print_Float(userdef23[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef23[1],6);
      Serial_Print("],");
    }      
    if (_24 == 1) {
      Serial_Print("\"get_userdef24\":[");
      Serial_Print_Float(userdef24[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef24[1],6);
      Serial_Print("],");
    }      
    if (_25 == 1) {
      Serial_Print("\"get_userdef25\":[");
      Serial_Print_Float(userdef25[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef25[1],6);
      Serial_Print("],");
    }      
    if (_26 == 1) {
      Serial_Print("\"get_userdef26\":[");
      Serial_Print_Float(userdef26[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef26[1],6);
      Serial_Print("],");
    }      
    if (_27 == 1) {
      Serial_Print("\"get_userdef27\":[");
      Serial_Print_Float(userdef27[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef27[1],6);
      Serial_Print("],");
    }      
    if (_28 == 1) {
      Serial_Print("\"get_userdef28\":[");
      Serial_Print_Float(userdef28[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef28[1],6);
      Serial_Print("],");
    }      
    if (_29 == 1) {
      Serial_Print("\"get_userdef29\":[");
      Serial_Print_Float(userdef29[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef29[1],6);
      Serial_Print("],");
    }      
    if (_30 == 1) {
      Serial_Print("\"get_userdef30\":[");
      Serial_Print_Float(userdef30[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef30[1],6);
      Serial_Print("],");
    }      
    if (_31 == 1) {
      Serial_Print("\"get_userdef31\":[");
      Serial_Print_Float(userdef31[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef31[1],6);
      Serial_Print("],");
    }      
    if (_32 == 1) {
      Serial_Print("\"get_userdef32\":[");
      Serial_Print_Float(userdef32[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef32[1],6);
      Serial_Print("],");
    }      
    if (_33 == 1) {
      Serial_Print("\"get_userdef33\":[");
      Serial_Print_Float(userdef33[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef33[1],6);
      Serial_Print("],");
    }      
    if (_34 == 1) {
      Serial_Print("\"get_userdef34\":[");
      Serial_Print_Float(userdef34[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef34[1],6);
      Serial_Print("],");
    }      
    if (_35 == 1) {
      Serial_Print("\"get_userdef35\":[");
      Serial_Print_Float(userdef35[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef35[1],6);
      Serial_Print("],");
    }      
    if (_36 == 1) {
      Serial_Print("\"get_userdef36\":[");
      Serial_Print_Float(userdef36[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef36[1],6);
      Serial_Print("],");
    }      
    if (_37 == 1) {
      Serial_Print("\"get_userdef37\":[");
      Serial_Print_Float(userdef37[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef37[1],6);
      Serial_Print("],");
    }      
    if (_38 == 1) {
      Serial_Print("\"get_userdef38\":[");
      Serial_Print_Float(userdef38[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef38[1],6);
      Serial_Print("],");
    }      
    if (_39 == 1) {
      Serial_Print("\"get_userdef39\":[");
      Serial_Print_Float(userdef39[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef39[1],6);
      Serial_Print("],");
    }      
    if (_40 == 1) {
      Serial_Print("\"get_userdef40\":[");
      Serial_Print_Float(userdef40[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef40[1],6);
      Serial_Print("],");
    }      
    if (_41 == 1) {
      Serial_Print("\"get_userdef41\":[");
      Serial_Print_Float(userdef41[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef41[1],6);
      Serial_Print("],");
    }      
    if (_42 == 1) {
      Serial_Print("\"get_userdef42\":[");
      Serial_Print_Float(userdef42[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef42[1],6);
      Serial_Print("],");
    }      
    if (_43 == 1) {
      Serial_Print("\"get_userdef43\":[");
      Serial_Print_Float(userdef43[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef43[1],6);
      Serial_Print("],");
    }      
    if (_44 == 1) {
      Serial_Print("\"get_userdef44\":[");
      Serial_Print_Float(userdef44[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef44[1],6);
      Serial_Print("],");
    }      
    if (_45 == 1) {
      Serial_Print("\"get_userdef45\":[");
      Serial_Print_Float(userdef45[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef45[1],6);
      Serial_Print("],");
    }      
    if (_46 == 1) {
      Serial_Print("\"get_userdef46\":[");
      Serial_Print_Float(userdef46[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef46[1],6);
      Serial_Print(",");
      Serial_Print_Float(userdef46[2],6);
      Serial_Print("],");
    }      
    if (_47 == 1) {
      Serial_Print("\"get_userdef47\":[");
      Serial_Print_Float(userdef47[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef47[1],6);
      Serial_Print(",");
      Serial_Print_Float(userdef47[2],6);
      Serial_Print("],");
    }      
    if (_48 == 1) {
      Serial_Print("\"get_userdef48\":[");
      Serial_Print_Float(userdef48[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef48[1],6);
      Serial_Print(",");
      Serial_Print_Float(userdef48[2],6);
      Serial_Print("],");
    }      
    if (_49 == 1) {
      Serial_Print("\"get_userdef49\":[");
      Serial_Print_Float(userdef49[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef49[1],6);
      Serial_Print(",");
      Serial_Print_Float(userdef49[2],6);
      Serial_Print("],");
    }      
    if (_50 == 1) {
      Serial_Print("\"get_userdef50\":[");
      Serial_Print_Float(userdef50[0],6);
      Serial_Print(",");
      Serial_Print_Float(userdef50[1],6);
      Serial_Print(",");
      Serial_Print_Float(userdef50[2],6);
      Serial_Print("],");
    }      
}

void add_userdef(int location, int num_of_objects, int bytes, int power_off) {
  
  float userdef;
  
  if (power_off == 1) {
    call_print_calibration(1);
    for (int i=0;i<num_of_objects;i++) {                        // loop "num_of_objects" times for objects which are "bytes" size for each object
      userdef = user_enter_dbl(60000);  
      EEPROM_writeAnything(location+i*bytes,userdef);
    }  
  }
  else if (power_off !=1) {
      
    for (int i=0;i<num_of_objects;i++) {                        // loop "num_of_objects" times for objects which are "bytes" size for each object
      userdef = user_enter_dbl(60000);  
      if (userdef > 0 && userdef < 5000) {                                                // in case pwr_off_ms is less than 5 seconds, set it to 2 minutes by default, otherwise leave it as whatever the user defined in EEPROM
        userdef = 120000;
      }
      else if (userdef < 0) {                                                               // in case pwr_off_ms is < 0 , then make it maximum time (functionally never turn off)
        userdef = 99999999;
      }
      Serial_Print_Float(userdef,6);
      EEPROM_writeAnything(location+i*bytes,userdef);
    }  
  }
  
  if (power_off == 1) {
    call_print_calibration(1);
  }
}

void calibrate_offset() {

  call_print_calibration(1);

  slope_34 = user_enter_dbl(60000);  
  EEPROM_writeAnything(24,slope_34);

  yintercept_34 = user_enter_dbl(60000);  
  EEPROM_writeAnything(28,yintercept_34);

  slope_35 = user_enter_dbl(60000);  
  EEPROM_writeAnything(32,slope_35);

  yintercept_35 = user_enter_dbl(60000);  
  EEPROM_writeAnything(36,yintercept_35);

  call_print_calibration(1);
}

void calibrate_light_sensor() {

  call_print_calibration(1);

  light_slope = user_enter_dbl(60000);  
  EEPROM_writeAnything(4,light_slope);

  light_y_intercept = user_enter_dbl(60000);  
  EEPROM_writeAnything(8,light_y_intercept);

  call_print_calibration(1);
}

float Contactless_Temperature(int var1) {
  if (var1 == 1 || var1 == 0) {
    float objt = readObjTempC_mod();
    float diet = tmp006.readDieTempC();
    delay(4000); // 4 seconds per reading for 16 samples per reading but shortened to make faster samples
#ifdef AVERAGES
    Serial_Print("\"object_temperature\": ");
    Serial_Print(objt);
    Serial_Print(",");  
    Serial_Print("\"board_temperature\": "); 
    Serial_Print(diet);
    Serial_Print(",");
#endif
    if (objt < -60 || objt > 1000) {                             // if the value is crazy probably indicating that there's no sensor installed
      objt_average = -99;
      objt = -99;
      Serial_Print("\"error\":\"no contactless temp sensor found\"");
      Serial_Print(",");
      return objt;
    }
    else {
      objt_average += objt / averages;
      return objt;
    }
  }
}

unsigned long Co2(int var1) {
  if (var1 == 1 || var1 == 0) {
    requestCo2(readCO2,2000);
    unsigned long co2_value = getCo2(response);
#ifdef DEBUGSIMPLE
    Serial_Print("\"co2_content\":");
    Serial_Print(co2_value);  
    Serial_Print(",");
#endif
    delay(100);
    co2_value_average += co2_value / averages;
    return co2_value;
  }
}

void requestCo2(byte packet[],int timeout) {
  int error = 0;
  long start1 = millis();
  long end1 = millis();
  while(!Serial3.available()) { //keep sending request until we start to get a response
    Serial3.write(readCO2,7);
    delay(50);
    end1 = millis();
    if (end1 - start1 > timeout) {
      Serial_Print("\"error\":\"no co2 sensor found\"");
      Serial_Print(",");
      int error = 1;
      break;
    }
  }
  switch (error) {
  case 0:
    while(Serial3.available() < 7 ) //Wait to get a 7 byte response
    {
      timeout++;  
      if(timeout > 10) {   //if it takes to long there was probably an error
#ifdef DEBUGSIMPLE
        Serial_Print_Line("I timed out!");
#endif
        while(Serial3.available())  //flush whatever we have
            Serial3.read();
        break;                        //exit and try again
      }
      delay(50);
    }
    for (int i=0; i < 7; i++) {
      response[i] = Serial3.read();    
#ifdef DEBUGSIMPLE
      Serial_Print_Line("I got it!");
#endif
    }  
    break;
  case 1:
    break;
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

float getFloatFromSerialMonitor(){                                               // getfloat function from Gabrielle Miller on cerebralmeltdown.com - thanks!
  char inData[20];  
  float f=0;    
  int x=0;  
  while (x<1){  
    String str;   
    if (Serial.available()) {
      delay(100);
      int i=0;
      while (Serial.available() > 0) {
        char  inByte = Serial.read();
        str=str+inByte;
        inData[i]=inByte;
        i+=1;
        x=2;
      }
      f = atof(inData);
      memset(inData, 0, sizeof(inData));  
    }
  }//END WHILE X<1  
  return f; 
}

double readObjTempC_mod(void) {
  double Tdie = tmp006.readRawDieTemperature();
  double Vobj = tmp006.readRawVoltage();
  Vobj *= 156.25;  // 156.25 nV per LSB
  Vobj /= 1000; // nV -> uV
  Vobj /= 1000; // uV -> mV
  Vobj /= 1000; // mV -> V
  Tdie *= 0.03125; // convert to celsius
  Tdie += 273.15; // convert to kelvin

#ifdef DEBUG
  Serial_Print("Vobj = "); 
  Serial_Print(Vobj * 1000000); 
  Serial_Print("uV");
  Serial_Print("Tdie = "); 
  Serial_Print(Tdie); 
  Serial_Print(" C");
  Serial_Print("tmp006 calibration value: ");
  Serial_Print(tmp006_cal_S);
#endif

  double tdie_tref = Tdie - TMP006_TREF;
  double S = (1 + TMP006_A1*tdie_tref + 
    TMP006_A2*tdie_tref*tdie_tref);
  S *= tmp006_cal_S;
  S /= 10000000;
  S /= 10000000;

  double Vos = TMP006_B0 + TMP006_B1*tdie_tref + 
    TMP006_B2*tdie_tref*tdie_tref;

  double fVobj = (Vobj - Vos) + TMP006_C2*(Vobj-Vos)*(Vobj-Vos);

  double Tobj = sqrt(sqrt(Tdie * Tdie * Tdie * Tdie + fVobj/S));

  Tobj -= 273.15; // Kelvin -> *C
  return Tobj;
}

void i2cWrite(byte address, byte count, byte* buffer)
{
  Wire.beginTransmission(address);
  byte i;
  for (i = 0; i < count; i++)
  {
#if ARDUINO >= 100
    Wire.write(buffer[i]);
#else
    Wire.send(buffer[i]);
#endif
  }
  Wire.endTransmission();
}

void i2cRead(byte address, byte count, byte* buffer)
{
  Wire.requestFrom(address, count);
  byte i;
  for (i = 0; i < count; i++)
  {
#if ARDUINO >= 100
    buffer[i] = Wire.read();
#else
    buffer[i] = Wire.receive();
#endif
  }
}

void set_device_info(int _set) {
  //  serial_bt_flush();
  Serial_Print("{\"device_name\":\"");
  Serial_Print(DEVICE_NAME);
  Serial_Print("\",\"device_id\":");
  Serial_Print_Float(device_id,0);
  Serial_Print(",\"device_version\":");
  Serial_Print_Float(device_version,2);
  Serial_Print(",\"device_firmware\":\"");
  Serial_Print_Float(DEVICE_FIRMWARE,2);
  Serial_Print("\",\"device_manufacture\":\"");
  Serial_Print_Float(manufacture_date,0);
  Serial_Print("\",");
  batt_level(0);
  Serial_Print(",");
  EEPROM_readAnything(1440,pwr_off_ms);
  print_cal_userdef("auto_power_off", pwr_off_ms,1,2);
  Serial_Print_CRC();

  if (_set == 1) {
    // please enter new device ID (integers only) followed by '+'
    device_id = user_enter_dbl(60000);
    if (device_id == -1) {
      goto device_end;
    }
    EEPROM_writeAnything(12,device_id);
    // please enter new device version (integers only) followed by '+'
    device_version = user_enter_dbl(60000);
    if (device_version == -1) {
      goto device_end;
    }
    EEPROM_writeAnything(16,device_version);
    // please enter new date of manufacture (yyyymm) followed by '+'   
    manufacture_date = user_enter_dbl(60000);
    if (manufacture_date == -1) {
      goto device_end;
    }
    EEPROM_writeAnything(20,manufacture_date);
    set_device_info(0);
  }
device_end:
  delay(1);
}

void serial_bt_flush() {
  while (Serial.available()>0) {                                                   // Flush incoming serial of the failed command
    Serial.read();
  }
  while (Serial1.available()>0) {                                                   // Flush incoming serial of the failed command
    Serial1.read();
  }
}

void configure_bluetooth () {

  // Bluetooth Programming Sketch for Arduino v1.2
  // By: Ryan Hunt <admin@nayr.net>
  // License: CC-BY-SA
  //
  // Standalone Bluetooth Programer for setting up inexpnecive bluetooth modules running linvor firmware.
  // This Sketch expects a bt device to be plugged in upon start. 
  // You can open Serial Monitor to watch the progress or wait for the LED to blink rapidly to signal programing is complete.
  // If programming fails it will enter command where you can try to do it manually through the Arduino Serial Monitor.
  // When programming is complete it will send a test message across the line, you can see the message by pairing and connecting
  // with a terminal application. (screen for linux/osx, hyperterm for windows)
  //
  // Hookup bt-RX to PIN 11, bt-TX to PIN 10, 5v and GND to your bluetooth module.
  //
  // Defaults are for OpenPilot Use, For more information visit: http://wiki.openpilot.org/display/Doc/Serial+Bluetooth+Telemetry

  Serial_Print("{\"response\": \"");
  // Enter bluetooth device name as seen by other bluetooth devices (20 char max), followed by '+'.
  String name = user_enter_str(60000,0);  
  // Enter current bluetooth device baud rate, followed by '+' (if new jy-mcu,it's probably 9600, if it's already had firmware installed by 115200)
  long baud_now = user_enter_dbl(60000);  
  // PLEASE NOTE - the pairing key has been set automatically to '1234'.

  int pin =         1234;                    // Pairing Code for Module, 4 digits only.. (0000-9999)
  int led =         13;                      // Pin of Blinking LED, default should be fine.
  char* testMsg =   "PhotosynQ changing bluetooth name!!"; //
  int x;
  int wait =        1000;                    // How long to wait between commands (1s), dont change this.

  pinMode(led, OUTPUT);
  Serial.begin(115200);                      // Speed of Debug Console

  // Configuring bluetooth module for use with PhotosynQ, please wait.
  Serial1.begin(baud_now);                        // Speed of your bluetooth module, 9600 is default from factory.
  digitalWrite(led, HIGH);                 // Turn on LED to signal programming has started
  delay(wait);
  Serial1.print("AT");
  delay(wait);
  Serial1.print("AT+VERSION");
  delay(wait);
  Serial.print("Setting PIN : ");          // Set PIN
  Serial.println(pin);
  Serial1.print("AT+PIN"); 
  Serial1.print(pin); 
  delay(wait);
  Serial.print("Setting NAME: ");          // Set NAME
  Serial.print(name);
  Serial1.print("AT+NAME");
  Serial1.print(name); 
  delay(wait);
  Serial.println("Setting BAUD: 115200");   // Set baudrate to 115200
  Serial1.print("AT+BAUD8");                   
  delay(wait);

  if (verifyresults()) {                   // Check configuration
    Serial_Print_Line("Configuration verified");
  } 
  digitalWrite(led, LOW);                 // Turn off LED to show failure.
  Serial_Print_Line("\"}");                  // close out JSON
  Serial_Print_Line("");
}

int verifyresults() {                      // This function grabs the response from the bt Module and compares it for validity.
  int makeSerialStringPosition;
  int inByte;
  char serialReadString[50];

  inByte = Serial1.read();
  makeSerialStringPosition=0;
  if (inByte > 0) {                                                // If we see data (inByte > 0)
    delay(100);                                                    // Allow serial data time to collect 
    while (makeSerialStringPosition < 38){                         // Stop reading once the string should be gathered (37 chars long)
      serialReadString[makeSerialStringPosition] = inByte;         // Save the data in a character array
      makeSerialStringPosition++;                                  // Increment position in array
      inByte = Serial1.read();                                          // Read next byte
    }
    serialReadString[38] = (char) 0;                               // Null the last character
    if(strncmp(serialReadString,bt_response,37) == 0) {               // Compare results
      return(1);                                                    // Results Match, return true..
    }
    Serial_Print("VERIFICATION FAILED!!!, EXPECTED: ");           // Debug Messages
    Serial_Print_Line(bt_response);
    Serial_Print("VERIFICATION FAILED!!!, RETURNED: ");           // Debug Messages
    Serial_Print_Line(serialReadString);
    return(0);                                                    // Results FAILED, return false..
  } 
  else {                                                                            // In case we haven't received anything back from module
    Serial_Print_Line("VERIFICATION FAILED!!!, No answer from the bt module ");        // Debug Messages
    Serial_Print_Line("Check your connections and/or baud rate");
    return(0);                                                                      // Results FAILED, return false..
  }
}

int calculate_intensity(int _light,int tcs_on,int _cycle,float _light_intensity) {  
#ifdef DEBUGSIMPLE
  Serial_Print("calculate intensity vars _light, tcs_on, _light_intensity, _tcs, cycle, act_intensities.getLong(_cycle), meas_intensities.getLong(_cycle), cal_intensities.getLong(_cycle)   ");
  Serial_Print(",");
  Serial_Print(_light);
  Serial_Print(",");
  Serial_Print(tcs_on);
  Serial_Print(",");
  Serial_Print(_cycle);
  Serial_Print(",");
  Serial_Print(_light_intensity);
  Serial_Print(",");
  Serial_Print(act_intensities.getLong(_cycle));
  Serial_Print(",");
  Serial_Print(meas_intensities.getLong(_cycle));
  Serial_Print(",");
  Serial_Print_Line(cal_intensities.getLong(_cycle));
#endif  

  int on = 0;                                                                            // so identify the places to turn the light on by flipping this to 1
  int _tcs = 0;
  if (_light == 2 || _light == 20) {                                                      // if it's a saturating light, and...
    if (act_intensities.getLong(_cycle) > 0) {                                            // if the actinic intensity is greater than zero then...
      on = 1;
      act_intensity = act_intensities.getLong(_cycle);                                    // turn light on and set intensity equal to the intensity specified in the JSON
    }
    else if (act_intensities.getLong(_cycle) < 0 && tcs_on > 0 && _light_intensity > 0) {   // if the intensity is -1 AND tcs_to_act is on AND the uE value _tcs_to_act is > 0 (ie ambient light is >0)
      on = 1;
      _tcs = (uE_to_intensity(_light,_light_intensity)*tcs_on)/100;
      act_intensity = _tcs;                                                                 // then turn light on, and set intensity to ambient
//      Serial_Print_Line(_light);
//      Serial_Print_Line(_tcs);
    }
  }

  else if (_light == 15 || _light == 16 || _light == 11 || _light == 12) {                     // if it's a measuring light, and...  
    if (meas_intensities.getLong(_cycle) > 0) {                                            // if the actinic intensity is greater than zero then...
      on = 1;
      meas_intensity = meas_intensities.getLong(_cycle);                                    // turn light on and set intensity equal to the intensity specified in the JSON
    }
    else if (meas_intensities.getLong(_cycle) < 0 && tcs_on > 0 && _light_intensity > 0) {      // if the intensity is -1 AND tcs_to_act is on AND the uE value _tcs_to_act is > 0 (ie ambient light is >0)
      on = 1;
      _tcs = (uE_to_intensity(_light,_light_intensity)*tcs_on)/100;
      meas_intensity = _tcs;                                                                 // then turn light on, and set intensity to ambient
//      Serial_Print_Line(_light);
//      Serial_Print_Line(_tcs);
    }
  }

  else if (_light == 14 || _light == 10) {                                                   // if it's a calibrating light, and...  
    if (cal_intensities.getLong(_cycle) > 0) {                                            // if the actinic intensity is greater than zero then...
      on = 1;
      cal_intensity = cal_intensities.getLong(_cycle);                                    // turn light on and set intensity equal to the intensity specified in the JSON
    }
    else if (cal_intensities.getLong(_cycle) < 0 && tcs_on > 0 && _light_intensity > 0) {      // if the intensity is -1 AND tcs_to_act is on AND the uE value _tcs_to_act is > 0 (ie ambient light is >0)
      on = 1;
      cal_intensity = _tcs;                                                       // then turn light on, and set intensity to ambient
    }
  }
  return on;
}


int calculate_intensity_background(int _light,int tcs_on,int _cycle,float _light_intensity, int _background_intensity) {    // calculate intensity of background light if it's set at a constant level by the user
#ifdef DEBUGSIMPLE
  Serial_Print("calculate background intensity vars _light, tcs_on, _light_intensity, _tcs, cycle, act_intensities.getLong(_cycle), meas_intensities.getLong(_cycle), cal_intensities.getLong(_cycle)   ");
  Serial_Print(",");
  Serial_Print(_light);
  Serial_Print(",");
  Serial_Print(tcs_on);
  Serial_Print(",");
  Serial_Print(_cycle);
  Serial_Print(",");
  Serial_Print(_light_intensity);
  Serial_Print(",");
  Serial_Print(act_intensities.getLong(_cycle));
  Serial_Print(",");
  Serial_Print(meas_intensities.getLong(_cycle));
  Serial_Print(",");
  Serial_Print(cal_intensities.getLong(_cycle));
#endif
  int on = 0;   // so identify the places to turn the light on by flipping this to 1
  int _tcs = 0;

  if (_light == 2 || _light == 20) {                                                      // if it's a saturating light, and...
    if (_background_intensity > 0) {                                            // if actinic background intensity is preset then
      dac.analogWrite(0,_background_intensity);                                 // set the actinic to that value
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
    else if (tcs_on > 0 && _light_intensity > 0) {                                       // or if tcs_to_act is on and ambient light is greater than zero then...
      _tcs = (uE_to_intensity(_light,_light_intensity)*tcs_on)/100;
//      Serial_Print_Line(_light);
//      Serial_Print_Line(_tcs);
      dac.analogWrite(0,_tcs);                                                           // set the actinic to that value      
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
  }
  else if (_light == 15 || _light == 16 || _light == 11 || _light == 12) {                       // if it's a measuring light, and...  
    if (_background_intensity > 0) {                                            // if actinic background intensity is preset then
      dac.analogWrite(3,_background_intensity);                                 // set the actinic to that value
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
    else if (tcs_on > 0 && _light_intensity > 0) {                                       // or if tcs_to_act is on and ambient light is greater than zero then...
      _tcs = (uE_to_intensity(_light,_light_intensity)*tcs_on)/100;
//      Serial_Print_Line(_light);
//      Serial_Print_Line(_tcs);
      dac.analogWrite(3,_tcs);                                                           // set the actinic to that value      
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
  }
  else if (_light == 14 || _light == 10) {                                                     // if it's a calibrating light, and...  
    if (_background_intensity > 0) {                                            // if actinic background intensity is preset then
      dac.analogWrite(2,_background_intensity);                                 // set the actinic to that value
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
    else if (tcs_on > 0 && _light_intensity > 0) {                                       // or if tcs_to_act is on and ambient light is greater than zero then...
      dac.analogWrite(2,_tcs);                                                           // set the actinic to that value      
      on = 1;                                                                            // so identify the places to turn the light on by flipping this to 1
    }
  }
  return on;
}

void reset_freq() {
  analogWriteFrequency(5, 187500);                                               // reset timer 0
  analogWriteFrequency(3, 187500);                                               // reset timer 1
  analogWriteFrequency(25, 488.28);                                              // reset timer 2
/*
Teensy 3.0              Ideal Freq:
16	0 - 65535	732 Hz	        366 Hz
15	0 - 32767	1464 Hz	        732 Hz
14	0 - 16383	2929 Hz	        1464 Hz
13	0 - 8191	5859 Hz	        2929 Hz
12	0 - 4095	11718 Hz	5859 Hz
11	0 - 2047	23437 Hz	11718 Hz
10	0 - 1023	46875 Hz	23437 Hz
9	0 - 511	        93750 Hz	46875 Hz
8	0 - 255	        187500 Hz	93750 Hz
7	0 - 127	        375000 Hz	187500 Hz
6	0 - 63	        750000 Hz	375000 Hz
5	0 - 31	        1500000 Hz	750000 Hz
4	0 - 15	        3000000 Hz	1500000 Hz
3	0 - 7	        6000000 Hz	3000000 Hz
2	0 - 3	        12000000 Hz	6000000 Hz

 */
}

// read 3 axis values (in milli-Gs) from a MMA8653FC accelerometer chip
// based on code from hoelig
// modifications by Jon Zeeff, 2015

// I2C BUS:  already defined in "wire" librairy
// SDA: PIN 2 with pull up 4.7K to 3.3V on arduino Micro
// SCL: PIN 3 with pull up 4.7K to 3.3V on arduino Micro
// Accelerometer connected to +3.3V of arduino DO NOT CONNECT TO 5V (this will destroy the accelerometer!)
// all GND Pin of accelerometer connected to gnd of arduino

/********************ACCELEROMETER DATAS************/

// adresss of accelerometer
#define ADDRESS 0X1D		// MMA8653FC and MMA8652FC
// address of registers for MMA8653FC
#define ctrl_reg1 0x2A
#define ctrl_reg2 0x2B
#define ctrl_reg3 0x2C
#define ctrl_reg4 0x2D
#define ctrl_reg5 0x2E
#define whoami_reg 0x0D
#define whoami_val 0x5A
#define int_source 0x0C
#define status_ 0x00
#define f_setup 0x09
#define out_x_msb 0x01
#define out_y_msb 0x03
#define out_z_msb 0x05
#define sysmod 0x0B
#define xyz_data_cfg 0x0E

//******PROGRAM DATAS**********/
#define ctrl_reg_address 0     // to understand why 0x00 and not 0x01, look at the data-sheet p.19 or on the comments of the sub. This is valid only becaus we use auto-increment   


//------------------------------------------------------------------

int MMA8653FC_init () {
  int i;
  
  I2C_SEND (ctrl_reg1, 0X00);	        // standby to be able to configure
  I2C_SEND (xyz_data_cfg, B00000000);	// 2G full range mode
  I2C_SEND (ctrl_reg1, B00000001);	// Output data rate at 800Hz, no auto wake, no auto scale adjust, no fast read mode

  if ((i = MMA8653_who_am_i()) == whoami_val)
     return 0;                          // OK
  else
     return -1;                         // error
}

//------------------------------------------------------------------

static void
I2C_SEND (unsigned char REG_ADDRESS, unsigned char DATA)	//SEND data
{
  Wire.beginTransmission (ADDRESS);
  Wire.write (REG_ADDRESS);
  Wire.write (DATA);
  Wire.endTransmission ();
  delayMicroseconds (2);
}	        

//------------------------------------------------------------------

// read current values from accelerometer in milli-Gs

void MMA8653FC_read(int *axeXnow, int *axeYnow, int *axeZnow) {

  Wire.beginTransmission (ADDRESS);	  //=ST + (Device Adress+W(0)) + wait for ACK
  Wire.write (ctrl_reg_address);	  // store the register to read in the buffer of the wire library
  Wire.endTransmission (I2C_NOSTOP,1000); // actually send the data on the bus -note: returns 0 if transmission OK-
  delayMicroseconds (2);	          // 
  Wire.requestFrom (ADDRESS, 7);	  // read a number of byte and store them in wire.read (note: by nature, this is called an "auto-increment register adress")
  // return a value in milli-Gs
  Wire.read();   // discard first byte (status)
  // read six bytes
  // note:  2G is 32768, -2G is -32768
  *axeXnow = ((int16_t)((Wire.read() << 8) || Wire.read()) * 1000) / (32768 / 2); // MSB first
  *axeYnow = ((int16_t)((Wire.read() << 8) || Wire.read()) * 1000) / (32768 / 2); // MSB first
  *axeZnow = ((int16_t)((Wire.read() << 8) || Wire.read()) * 1000) / (32768 / 2); // MSB first
}  // MMA8653FC_read()

//------------------------------------------------------------------


// read the chip whoami value

int MMA8653_who_am_i (void) {
  Wire.beginTransmission (ADDRESS);	  // transmit to device 0x0E
  Wire.write (whoami_reg);		  // x MSB reg
  Wire.endTransmission(I2C_NOSTOP);       // stop transmitting
  delayMicroseconds (2);	          // 
  Wire.requestFrom (ADDRESS, 1);	  // request 1 byte
  return (unsigned char)Wire.read();      // read the byte
}


/*
  MAG3110 (i2c compass) read code
  
  JZ, May 2015
  note: returns units of tenths of a micro-tesla but is uncalibrated for offsets (ie, wildly off)
        value will always range +/- 20,000
        Earth's magnetic field is 25 to 65 micro-teslas or a range of 250-650 tenths.
        
  
*/
// note: normally a compass chip is calibrated with offsets learned from (max-min)/2 as you rotate it
// this chip can apply these offsets

#define MAG_ADDR  0x0E		// 7-bit address for the MAG3110, doesn't change

int MAG3110_init (void) {
  delay(1);

  Wire.beginTransmission (MAG_ADDR);	// transmit to device 0x0E
  Wire.write (0x10);		        // cntrl register1
  Wire.write (0B00101001);              // some oversampling, active mode
  Wire.write (0B10100000);		// reg 2 - enable auto resets and raw
  Wire.endTransmission(I2C_STOP, 1000);

  delay(1);

  if (MAG3110_who_am_i() != 0XC4)
     return -1;                         // error
  else
     return 0;                          // chip detected
}


int MAG3110_who_am_i (void) {
  Wire.beginTransmission (MAG_ADDR);	// transmit to device 0x0E
  Wire.write (0x07);		        // x MSB reg
  Wire.endTransmission(I2C_STOP, 1000);	// stop transmitting

  delayMicroseconds (2);	        // needs 1.3us free time between start and stop

  Wire.requestFrom (MAG_ADDR, 1);	// request 1 byte
  delay(2);
  return (unsigned char)Wire.read();    // read the byte
}

// return temp in degrees C.  Note, uncalibrated offset is way off, so use only as a relative reading

int MAG3110_temp(void) {
  Wire.beginTransmission (MAG_ADDR);	// transmit to device 0x0E
  Wire.write (0x0f);		        // register for temp 
  Wire.endTransmission(I2C_STOP, 1000);	// stop transmitting

  delayMicroseconds (2);	        // needs 1.3us free time between start and stop

  Wire.requestFrom (MAG_ADDR, 1);	// request 1 byte
  delay(2);
  return (signed char)Wire.read();	// read the byte
}


// read all three axis

void MAG3110_read (int *x,int *y,int *z) {
  Wire.beginTransmission (MAG_ADDR);    // transmit to device 0x0E
  Wire.write (1);                       // starting register is 0x01
  Wire.endTransmission(I2C_STOP, 1000); // stop transmitting

  delayMicroseconds (2);                // needs 1.3us free time between start and stop

  Wire.requestFrom (MAG_ADDR, 6);       // request 6 bytes (all 3 values)

  unsigned char high_byte=0, low_byte=0;    // MSB and LSB

  *x = *y = *z = 0;

  if (Wire.available())
     high_byte = Wire.read();           // read the byte
  if (Wire.available())
     low_byte = Wire.read();            // read the byte
  *x = (int16_t) (low_byte || (high_byte << 8));    // concatenate the MSB and LSB

  if (Wire.available())
     high_byte = Wire.read();           // read the byte
  if (Wire.available())
     low_byte = Wire.read();            // read the byte
  *y = (int16_t) (low_byte || (high_byte << 8));    // concatenate the MSB and LSB

  if (Wire.available())
     high_byte = Wire.read();           // read the byte
  if (Wire.available())
     low_byte = Wire.read();            // read the byte
  *z = (int16_t) (low_byte || (high_byte << 8));    // concatenate the MSB and LSB

} // MAG3110_read()

float MLX90614_Read(int TaTo) {
        I2C0_F = 0x85; // 400k 120
        delay(5);
        int rawData = MLX90614_getRawData(TaTo);
        double tempData = (rawData * 0.02) - 0.01;
        tempData -= 273.15;
        I2C0_F = 0x0B; // 1.2M 40
        delay(5);

        return tempData;
}

int MLX90614_getRawData(int TaTo) {

        // Store the two relevant bytes of data for temperature
        byte dataLow = 0x00;
        byte dataHigh = 0x00;
        Wire.beginTransmission(0x5A);
        if (TaTo)
                Wire.send(0x06); //measure ambient temp
        else
                Wire.send(0x07); // measure objec temp
        Wire.endTransmission(I2C_NOSTOP);

        Wire.requestFrom((uint8_t) 0x5A, (size_t) 2);
        dataLow = Wire.read();
        dataHigh = Wire.read();
        Wire.endTransmission();
        int tempData = (((dataHigh & 0x007F) << 8) + dataLow);
        return tempData;
}
 
void readSpectrometer()
{
/*
  //int delay_time = 35;     // delay per half clock (in microseconds).  This ultimately conrols the integration time.
  int delay_time = 1;     // delay per half clock (in microseconds).  This ultimately conrols the integration time.
  int idx = 0;
  int read_time = 35;      // Amount of time that the analogRead() procedure takes (in microseconds)
  int intTime = 100; 
  int accumulateMode = false;
*/
  int i;

  // Step 1: start leading clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

  // Step 2: Send start pulse to signal start of integration/light collection
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);

  // Step 3: Integration time -- sample for a period of time determined by the intTime parameter
  int blockTime = delay_time * 8;
  int numIntegrationBlocks = (intTime * 1000) / blockTime;
  for (int i = 0; i < numIntegrationBlocks; i++) {
    // Four clocks per pixel
    // First block of 2 clocks -- measurement
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);

    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }


  // Step 4: Send start pulse to signal end of integration/light collection
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);

  // Step 5: Read Data 2 (this is the actual read, since the spectrometer has now sampled data)
  idx = 0;
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    // Four clocks per pixel
    // First block of 2 clocks -- measurement
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);

    // Analog value is valid on low transition
    if (accumulateMode == false) {
      spec_data[idx] = analogRead(SPEC_VIDEO);
      spec_data_average[idx] += spec_data[idx];
    } else {
      spec_data[idx] += analogRead(SPEC_VIDEO);
    }
    idx += 1;
    if (delay_time > read_time) delayMicroseconds(delay_time - read_time);   // Read takes about 135uSec

    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);

    // Second block of 2 clocks -- idle
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

  // Step 6: trailing clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }
}

void print_data()
{
  Serial_Print("\"data_raw\":[");
  for (int i = 0; i < SPEC_CHANNELS; i++) 
  {
    Serial_Print_Float(spec_data[i],4);
    if (i != SPEC_CHANNELS-1) {                 // if it's the last one in printed array, don't print comma
      Serial_Print(",");
    }
  }
  Serial_Print("]");
}

