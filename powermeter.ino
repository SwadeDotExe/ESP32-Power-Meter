/*
  Debug LED Color Code:
      Blinking Blue - Connecting to WiFi
      Blinking Green - Connected to database
      Blinking Red - Failed to reach database
      Solid Blue - Running measurements
      Solid Green - Successfully wrote to database
      Solid Red - Failed to write to database
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define DEVICE "ESP32"
#include "EmonLib.h"
#include <InfluxDbClient.h>

// Brownout Detector Disable
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Wifi + InfluxDB Details
const char* ssid = "RHIT-OPEN";
const char* password = "";
#define INFLUXDB_URL "http://monitoringserver.reshall.rose-hulman.edu:8086"
#define INFLUXDB_DB_NAME "Sensors"
#define INFLUXDB_USER "sensor"
#define INFLUXDB_PASSWORD "bW7jxSOqoTLhgfdsddcXSq5WLHumeGPUp"

// LED Pins
const int redLED = 25;
const int blueLED = 26;
const int greenLED = 27;

// Temperature Sensor Pin
const int tempPin = 34;

// WiFi Hostname (For Uptime Monitoring)
String hostname = "powermeter";

// Influx DB client
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

// CT Clamps
EnergyMonitor emon1;   // Current 1
EnergyMonitor emon2;   // Current 2
EnergyMonitor emon3;   // Voltage

// Number of Samples for Averaging Function
const int numReadings = 50;

// CT Clamp 1 Averaging Variables
double readings1[numReadings];      // the readings from the analog input
int readIndex1 = 0;                 // the index of the current reading
double total1 = 0;                  // the running total
double current1 = 0;                // the average

// CT Clamp 2 Averaging Variables
double readings2[numReadings];      // the readings from the analog input
int readIndex2 = 0;                 // the index of the current reading
double total2 = 0;                  // the running total
double current2 = 0;                // the average

// Temperature Sensor Averaging Variables
double readings3[numReadings];      // the readings from the analog input
int readIndex3 = 0;                 // the index of the current reading
double total3 = 0;                  // the running total
double tempAvg = 0;                 // the average

// Voltage Sensor Averaging Variables
double readings4[numReadings];      // the readings from the analog input
int readIndex4 = 0;                 // the index of the current reading
double total4 = 0;                  // the running total
double voltAvg = 0;                 // the average

// Data point
Point sensor("wifi_status");

// Wifi Define
WiFiClient espClient;

void setup()
{

  Serial.begin(9600);
  //disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // LED Setup
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // LED Test
  digitalWrite(redLED, HIGH);
  delay(250);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  delay(250);
  digitalWrite(greenLED, LOW);
  digitalWrite(blueLED, HIGH);
  delay(250);
  digitalWrite(blueLED, LOW);
  delay(500);

  //Connect to WiFi
  setup_wifi();

  // Set InfluxDB 1 authentication params
  client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);

  // Add constant tags - only once
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
    digitalWrite(greenLED, HIGH);
    delay(100);
    digitalWrite(greenLED, LOW);
    delay(100);
    digitalWrite(greenLED, HIGH);
    delay(100);
    digitalWrite(greenLED, LOW);
    delay(100);
    digitalWrite(greenLED, HIGH);
    delay(100);
    digitalWrite(greenLED, LOW);
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    digitalWrite(redLED, HIGH);
    delay(100);
    digitalWrite(redLED, LOW);
    delay(100);
    digitalWrite(redLED, HIGH);
    delay(100);
    digitalWrite(redLED, LOW);
    delay(100);
    digitalWrite(redLED, HIGH);
    delay(100);
    digitalWrite(redLED, LOW);
  }

  // Initalize Current Clamps
  emon1.current(36, 18.65);             // Current Clamp 1: input pin, calibration.
  emon2.current(39, 18.65);             // Current Clamp 2: input pin, calibration.

  // Initalize Voltage Sensor
  emon3.voltage(35, 46.8, 1.7);  // Voltage: input pin, calibration, phase_shift
}

void loop()
{

  // Reset debug LEDs
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, HIGH);

  // Read Sensors and Average Results
  for (int a = 0; a < numReadings; a++) {
    readSensors();
  }

  /***********************************/
  //                                 //
  //        Data Processing          //
  //                                 //
  /***********************************/

  digitalWrite(blueLED, LOW);

  // Store measured value into point
  sensor.clearFields();
  sensor.addField("CT1", current1);
  sensor.addField("CT2", current2);
  sensor.addField("Temperature", tempAvg);
  sensor.addField("Voltage", voltAvg);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
    digitalWrite(redLED, HIGH);
    delay(1000);
    digitalWrite(redLED, LOW);
  } else {
    digitalWrite(greenLED, HIGH);
    delay(1000);
    digitalWrite(greenLED, LOW);
  }

  //Wait 4s (plus 1s in previous command)
  delay(4000);
}

void readSensors() {

  /***********************************/
  //                                 //
  //       Current Clamp 1           //
  //                                 //
  /***********************************/

  // subtract the last reading:
  total1 = total1 - readings1[readIndex1];
  // read from the sensor:
  readings1[readIndex1] = emon1.calcIrms(1480);
  // add the reading to the total:
  total1 = total1 + readings1[readIndex1];
  // advance to the next position in the array:
  readIndex1 = readIndex1 + 1;
  // if we're at the end of the array...
  if (readIndex1 >= numReadings) {
    // ...wrap around to the beginning:
    readIndex1 = 0;
  }
  // calculate the average:
  current1 = total1 / numReadings;
  //   current1 = emon1.calcIrms(1480);

  /***********************************/
  //                                 //
  //       Current Clamp 2           //
  //                                 //
  /***********************************/

  // subtract the last reading:
  total2 = total2 - readings2[readIndex1];
  // read from the sensor:
  readings2[readIndex1] = emon2.calcIrms(1480);
  // add the reading to the total:
  total2 = total2 + readings2[readIndex1];
  // advance to the next position in the array:
  readIndex2 = readIndex2 + 1;
  // if we're at the end of the array...
  if (readIndex2 >= numReadings) {
    // ...wrap around to the beginning:
    readIndex2 = 0;
  }
  // calculate the average:
  current2 = total2 / numReadings;

  //current2 = emon2.calcIrms(1480);


  /***********************************/
  //                                 //
  //      Temperature Sensor         //
  //                                 //
  /***********************************/

  //Voltage reading from the temperature sensor
  int reading = analogRead(tempPin);

  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = reading;
  voltage /= 1024.0;
  double temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
  double temperatureF = (temperatureC * 9.0 / 5.0) + 36.0;

  // subtract the last reading:
  total3 = total3 - readings3[readIndex1];
  // read from the sensor:
  readings3[readIndex1] = temperatureF;
  // add the reading to the total:
  total3 = total3 + readings3[readIndex1];
  // advance to the next position in the array:
  readIndex3 = readIndex3 + 1;
  // if we're at the end of the array...
  if (readIndex3 >= numReadings) {
    // ...wrap around to the beginning:
    readIndex3 = 0;
  }
  // calculate the average:
  tempAvg = total3 / numReadings;

  //  tempAvg = temperatureF;

  /***********************************/
  //                                 //
  //        Voltage Sensor           //
  //                                 //
  /***********************************/
  // emon3.calcVI(20,2000);
  // subtract the last reading:
  total4 = total4 - readings4[readIndex4];
  // read from the sensor:
  readings4[readIndex4] = emon3.Vrms;
  // add the reading to the total:
  total4 = total4 + readings4[readIndex4];
  // advance to the next position in the array:
  readIndex4 = readIndex4 + 1;
  // if we're at the end of the array...
  if (readIndex4 >= numReadings) {
    // ...wrap around to the beginning:
    readIndex4 = 0;
  }
  // calculate the average:
  voltAvg = total4 / numReadings;

  delay(1);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(blueLED, HIGH);
    Serial.print(".");
    delay(250);
    digitalWrite(blueLED, LOW);
    delay(250);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(blueLED, LOW);
  delay(500);
}
