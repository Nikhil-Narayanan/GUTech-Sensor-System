#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //set lcd
const int chipSelect = 10; //cs or the save select pin from the sd shield is connected to 10.
RTC_DS1307 RTC;
float celsius, fahrenheit;
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; // Calculated litres/hour
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;

OneWire  ds(8);  // temperature senor on pin 8 (a 4.7K resistor is necessary) //

File dataFile;
DateTime now;
void flow () // Interrupt function
{
   flow_frequency++;
}
void setup(void) {
  lcd.begin(20, 4); //set LCD display to have 20 columns and 4 rows
  //Variable setup for flowsensor
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(0, flow, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
   // Variable setup for datalogger
  Serial.begin(9600);
  //setup clock
  Wire.begin();
  RTC.begin();
//check or the Real Time Clock is on
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    lcd.print("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // uncomment it & upload to set the time, date and start run the RTC!
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
//setup SD card
   lcd.setCursor(0,1);
   lcd.print("Initializing SD card...");
   Serial.print("Initializing SD card...");
  // see if the SD card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0,2);
    lcd.print("Card failed, or not present");
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  lcd.setCursor(0,2);
  lcd.print("card initialized.");
 
  //write down the date (year / month / day         prints only the start, so if the logger runs for sevenal days you only findt the start back at the begin.
    now = RTC.now();
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    dataFile.print("Start logging on: ");
    dataFile.print(now.year(),DEC);
    dataFile.print('/');
    dataFile.print(now.month(),DEC);
    dataFile.print('/');
    dataFile.print(now.day(),DEC);
    dataFile.println(" ");
    dataFile.println("Time,Temp Output [Celsius],Temp Input [Celsius],Temp Excanger [Celsius],Glycole Flow [L/hour]");
    dataFile.close();

}

void loop(void) {
// Temperature Sensor
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures

  //FlowSensor Loop
  currentTime = millis();
   // Every second, calculate and print litres/hour
   
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
   }
   // end flowsensor loop
 

//read the time
  now = RTC.now();
 
  //open file to log data in.
   dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  // log the temperature and time.
  if (dataFile) {

    dataFile.print(now.hour(),DEC);
    dataFile.print(":");
    dataFile.print(now.minute(),DEC);
    dataFile.print(":");
    dataFile.print(now.second(),DEC);
    dataFile.print(",");

//Sensor 0 = Behind Pump // Sensor 1 = Collector Output // Sensor 2 = Inside Heat Excanger

    dataFile.print(sensors.getTempCByIndex(1));
    dataFile.print(",");

    dataFile.print(sensors.getTempCByIndex(0));
    dataFile.print(",");

    dataFile.print(sensors.getTempCByIndex(2));
    dataFile.print(",");
   
    dataFile.print(l_hour);
    dataFile.println();
   
    dataFile.close();
    // print to the serial port too:
    Serial.println("data stored");
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
 

  Serial.print("Time is ");
  Serial.print(now.hour(),DEC);
  Serial.print(":");
  Serial.print(now.minute(),DEC);
  Serial.print(":");
  Serial.println(now.second(),DEC);

  lcd.setCursor(0,0);
  lcd.print("Temp Out is ");//shortened to fit in screen
  lcd.print(sensors.getTempCByIndex(1));
  lcd.print(" C");
  Serial.print("Temperature Collector Output is ");
  Serial.print(sensors.getTempCByIndex(1));
  Serial.println(" Celsius");

  lcd.setCursor(0,1);
  lcd.print("Temp In is ");
  lcd.print(sensors.getTempCByIndex(0));
  lcd.print(" C");
  Serial.print("Temperature Collector Input is ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.println(" Celsius");

  lcd.setCursor(0,2);
  lcd.print("Temp Exchange=" + sensors.getTempCByIndex(2) + "C");
  Serial.print("Temperature Inside Heat Excanger is ");
  Serial.print(sensors.getTempCByIndex(2));
  Serial.println(" Celsius");
 
  lcd.setCursor(0,3);
  lcd.print("Glyc Flow is ");
  lcd.print(1_hour, DEC);
  lcd.print("L/hour");
  Serial.print("Glycole flow is ");
  Serial.print(l_hour, DEC);
  Serial.println(" L/hour");
  Serial.println();
  delay(1000);
}
