#include "SoftwareSerial.h"
#include <SPI.h>
#include <SD.h>

// TODO
// Check that panelPins are assigned correctly
// Check that the pump timing is working correctly

// Initialize variables to store CO2 reading
SoftwareSerial cots1(7, 8); // Set up a virtual serial port using pin 12 for Rx and pin 13 for Tx
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  // Command packet to read CO2
byte response[] = {0, 0, 0, 0, 0, 0, 0}; // Create an array to store the response
int valMultiplier = 10; // CO2 conversion factor to ppm

// Set timer variables
long initMillis;
long pumpMillis = 0;
long previousMillis = 0;
long pumpTime = 0;

// Set pins
int pumpPin = 2;
int panelPin1 = 3;
int panelPin2 = 5;

// Set initial PWM duty cycle
float lightLevel = 0.50; // 0.50 = 50%
float u = 0;

// Set PID variables
int ref = 800; // Reference CO2 concentration (ppm)
int err = 0;
//int cErr = 0;
float K_p = 0.2/100; // => full brightness at 1300 ppm
//float K_i = 0.1; // CHANGE AFTER UPDATING CONTROLLER

// Initalize data file name
String dataFile;

void setup() {
  // Serial setup
  Serial.begin(9600); // Open the main serial port to communicate with the computer
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  Serial.println("\nSerial open.");
  cots1.begin(9600);  // Open the virtual serial port with a baud of 9600

  // Set panels to inital light level
  analogWrite(panelPin1, lightLevel * 255);
  analogWrite(panelPin2, lightLevel * 255);

  // Initialize peristaltic pump
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW); // Make sure pump is off

  // Initialize SD card (from arduino.cc)
  Serial.println("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present. Halting.");
    while (1);  // Don't do anything more
  }
  Serial.println("Card initialized, please enter log file name:");
  while (!Serial.available());
  dataFile = Serial.readStringUntil('\n') + ".csv";
  Serial.println("Thank you, beginning data collection.\n");

  Serial.println("Type a decimal between 0 and 1 to set the light level.");
  Serial.println("Type a number between 5 and 480 to activate the CO2 pump for that many seconds.\n");

  initMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis() - initMillis;

  // Shut off pump if enough time has lapsed
  if (pumpMillis != 0 && currentMillis - pumpMillis > pumpTime*1000) {
    digitalWrite(pumpPin, LOW);
    pumpTime = 0;
    pumpMillis = 0;
  }

  // Run the following once a second
  if (currentMillis - previousMillis > 1000) {
    previousMillis = currentMillis;

    String dataString = String(currentMillis / 1000) + ", ";

    // Call external function to get CO2 reading
    sendRequest(readCO2);
    unsigned long valCO2 = getValue(response);
    dataString += String(valCO2) + ", ";

    // Proportional control
    err = valCO2 - ref; // y-r
//    cErr += err*1; // integral of err
    u = K_p*err; // + K_i*cErr;
    if (u > 1) {
      u = 1;
    } else if (u < 0) { // consider changing to 20% so light never goes off
      u = 0;
    }
    lightLevel = u;
    analogWrite(panelPin1, u * 255);
    analogWrite(panelPin2, u * 255);

    if (Serial.available()) {
      float in = Serial.parseFloat();

      if (in >= 5 && in <= 480) {
        digitalWrite(pumpPin, HIGH);
        pumpTime = in;
        pumpMillis = currentMillis;
        Serial.print("Pump on until t = ");
        Serial.println((currentMillis/1000)+pumpTime);
      }
    }

    dataString += String(lightLevel);

    Serial.println(dataString); // Print to monitor: time (s), CO2 level (ppm), light level

    // Save data to SD card
    File dat = SD.open(dataFile, FILE_WRITE);
    if (dat) {
      dat.println(dataString);
      dat.close();
    } else {
      Serial.println("Error opening datalog file.");
    }
  } 
}
