/*
 * PDrone Example UPDTest.ino
 *
 * This sketch implements a simple UDP client that sends UDP packets
 * to a UDP server.
 *
 * Debug information is logged to Serial, WiFly assumed to be on Serial1 (Fio v3)
 *
 * This sketch is released to the public domain.
 *
 */

// WiFlyHQ allows us to have pretty code for setting up the connections
#include <WiFlyHQ.h>
// SoftwareSerial is used to communicate with the GPS
#include <SoftwareSerial.h>
// Wire lets us talk with the gyro
#include <Wire.h>
// Gyro lib
#include <L3G.h>
// GPS lib
#include<TinyGPS++.h>

/* Change these to match your WiFi network */
const char mySSID[] = "DRONE";
const char myPassword[] = "PHEONIXY";

void terminal();

// Wifly object
WiFly wifly;

// Gyro object
L3G gyro;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
static const int RXPin = 10, TXPin = 5;
SoftwareSerial ss(RXPin, TXPin);

// Baud rates
static const uint32_t GPSBaud = 9600;
static const uint32_t WIFLYBaud = 9600;
static const uint32_t USBBaud = 115200;

void setup()
{
  char buf[32];
  Serial.begin(USBBaud); // USB Connection
  Serial1.begin(WIFLYBaud); // WIFLY Connection
  ss.begin(GPSBaud); // GPS Connection
  delay(2000); // Allow time for serial monitor to be opened
  Serial.println("Starting");
  Serial.print("Free memory: ");
  Serial.println(wifly.getFreeMemory(),DEC);

  if (!wifly.begin(&Serial1, &Serial)) {
    Serial.println("Failed to start wifly");
    terminal();
  }

  if (wifly.getFlushTimeout() != 10) {
    Serial.println("Restoring flush timeout to 10msecs");
    wifly.setFlushTimeout(10);
    wifly.save();
    wifly.reboot();
  }

  /* Setup the WiFly to connect to a wifi network */
  Serial.println("Joining network");
  wifly.setSSID(mySSID);
  wifly.setPassphrase(myPassword);
  wifly.enableDHCP();

  if (wifly.join()) {

    Serial.println("Joined wifi network");

  } 
  else {

    Serial.println("Failed to join wifi network");
    terminal();

  }

  /* Setup for UDP packets, sent automatically */
  wifly.setIpProtocol(WIFLY_PROTOCOL_UDP);
  wifly.setHost("192.168.1.241", 55056);	// Send UDP packet to this server and port
  wifly.setDeviceID("PHEONIX");

  Serial.print("MAC: ");
  Serial.println(wifly.getMAC(buf, sizeof(buf)));
  Serial.print("IP: ");
  Serial.println(wifly.getIP(buf, sizeof(buf)));
  Serial.print("Netmask: ");
  Serial.println(wifly.getNetmask(buf, sizeof(buf)));
  Serial.print("Gateway: ");
  Serial.println(wifly.getGateway(buf, sizeof(buf)));
  Serial.print("DeviceID: ");
  Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

  Serial.println("WiFly ready");

}

//uint32_t p; //X Rotation (Pitch?)
//uint32_t r; //Y Rotation (Roll?)
//uint32_t y; //Z Rotation (Yaw?)
uint32_t e=0; //Elevator
//uint32_t r=0; //Rudder

void loop()
{

  //Read Gyro
  gyro.read();

  //Read GPS
  if(ss.available() > 0){
    gps.encode(ss.read());
  }

  //Read Input from wifly (Cached Data)
  if (wifly.available() > 0) {
    Serial.write(wifly.read());
  }  

  //Respond - Consider GPS, Gyro and request.


  //Send stuff to controller
  sendData();

}

void sendData() {

  wifly.print("XR:");  //X Rotation
  wifly.print(gyro.g.x);
  wifly.print("\tYR:");  //Y Rotation
  wifly.print(gyro.g.y);
  wifly.print("\tZR:");  //Z Rotation
  wifly.print(gyro.g.z);
  wifly.print("\tML:");  //Millisecond update
  wifly.print(lastUpdate);
  wifly.print("\tLA:");  //Latitude
  wifly.print(gps.location.lat());
  wifly.print("\tLG:");  //Longitude
  wifly.print(gps.location.lng());
  wifly.print("\tAT:");  //Altitude (m)
  wifly.print(gps.altitude.meters());
  wifly.print("\tNS:");  //Number of Satellites
  wifly.print(gps.satellites.value());
  wifly.print("\tHP:");  //HDOP (m)
  wifly.print(gps.hdop.value());
  wifly.print("\tSD:");  //Speed of sensor(km/hr)
  wifly.print(gps.speed.kmph());
  wifly.print("\tCH:1");  //Check - Verify complete message

  wifly.print("\r");

}

void terminal()
{
  Serial.println("Terminal ready");
  while (1) {
    if (wifly.available() > 0) {
      Serial.write(wifly.read());
    }

    if (Serial.available()) {
      wifly.write(Serial.read());
    }
  }
}