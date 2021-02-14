
#include "SoftwareSerial.h"
#include <SPI.h>
#include <SD.h>

// modified to comment out datalogging functionality

// CO2 sensor
// SoftwareSerial cots1(12, 13); //(Mega) Sets up a virtual serial port using pin 12 for Rx and pin 13 for Tx
SoftwareSerial cots1(7, 8); // (Nano) Sets up a virtual serial port using pin 12 for Rx and pin 13 for Tx
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0, 0, 0, 0, 0, 0, 0}; //create an array to store the response
int valMultiplier = 10;
float lightLevel = 5; // set PWM to 50% duty cycle => LEDs at half brightness
String n;
long initMillis;
long O2Millis = 0;
long CO2Millis = 0;

// O2 sensor
float Vout = 0;
const float VRefer = 5;       // voltage of adc reference
const int pinAdc   = A0;

long previousMillis = 0;

void setup() {
  // CO2 sensor
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  Serial.println("serial open");
  cots1.begin(9600);    //Opens the virtual serial port with a baud of 9600
  analogWrite(5, lightLevel * 25.5);
  analogWrite(6, lightLevel * 25.5);

  analogWrite(2, 0); // close oxygen
  analogWrite(3, 0); // close carbon dioxide

//  // initialize SD card (from arduino.cc)
//  Serial.print("Initializing SD card...");
//  // see if the card is present and can be initialized:
//  if (!SD.begin(10)) {
//    Serial.println("Card failed, or not present");
//    // don't do anything more:
//    while (1);
//  }
//  Serial.println("card initialized, please enter log file name:");
//  while(!Serial.available());
//  n = Serial.readStringUntil('\n') + ".csv";
//  Serial.println("thank you.\n");
  initMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis() - initMillis;

  if (O2Millis != 0 && currentMillis - O2Millis > 5000) {
    analogWrite(3, 0);
    O2Millis = 0;
  }
  if (CO2Millis != 0 && currentMillis - CO2Millis > 5000) {
    analogWrite(2, 0);
    CO2Millis = 0;
  }

  if (currentMillis - previousMillis > 1000) {
    previousMillis = currentMillis;

    String dataString = String(currentMillis/1000) + ",";
    File dat = SD.open(n, FILE_WRITE);

    // CO2 sensor
    sendRequest(readCO2);
    unsigned long valCO2 = getValue(response);
    dataString += String(valCO2) + ",";

    // O2 sensor
    double correctionFactor = 21.0 / 11.495602;
    dataString += String(correctionFactor * readConcentration()) + ",";
    
    if (Serial.available()) {
      float in = Serial.readStringUntil('\n').toFloat();
      if (in > 0 && in <= 10) {
        Serial.print("Light level: ");
        Serial.println(in);
        analogWrite(5, in * 25.5);
        analogWrite(6, in * 25.5);
        lightLevel = in;
      } else if (in == 100) {
        analogWrite(3, 255); // actuate oxygen
        O2Millis = currentMillis;
      } else if (in = 200) {
        analogWrite(2, 255); // actuate carbon dioxide
        CO2Millis = currentMillis;
      }
    }

    dataString += String(lightLevel);
    
    Serial.println(dataString);
//    if (dat) {
//      dat.println(dataString);
//      dat.close();
//      // print to the serial port too:
//    }
//    // if the file isn't open, pop up an error:
//    else {
//      Serial.println("error opening datalog.txt");
//    }

  }

}
