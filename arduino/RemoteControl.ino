/*
 * PDrone Example RemoteControl.ino
 *
 * This sketch implements a simple UDP client that sends UDP packets
 * to a UDP server.
 *
 * Debug information is logged to Serial, WiFly assumed to be on Serial1 (Fio v3)
 *
 * This sketch is released to the public domain.
 *
 * Further info on the setup used coming shortly.
 *
 */

// WiFlyHQ allows us to have pretty code for setting up the connections
#include <WiFlyHQ.h>
// Wire lets us talk with the gyro
#include <Wire.h>
// Gyro lib
#include <L3G.h>
// GPS lib
#include <TinyGPS++.h>
// Servo lib
#include <Servo.h>

/* Change these to match your WiFi network */
const char mySSID[] = "linksys";
const char myPassword[] = " ";

void terminal();

// Wifly object
WiFly wifly;

// Gyro object
L3G gyro;

// The TinyGPS++ object
TinyGPSPlus gps;

/////////////////////////
// Update Rate Control //
/////////////////////////
// Phant limits you to 10 seconds between posts. Use this variable
// to limit the update rate (in milliseconds):
const unsigned long UPDATE_RATE = 1000; // 100ms
unsigned long lastUpdate = 0; // Keep track of last update time

// Baud rates
static const uint32_t WIFLYBaud = 9600;
static const uint32_t USBBaud = 9600;

//Servos
Servo rudder; 
Servo elevator;
Servo leftMO;
Servo rigtMO;

static const uint32_t ruddpin = 6;
static const uint32_t elevpin = 4;
static const uint32_t leftpin = 8;
static const uint32_t rigtpin = 9;

void setup()
{
  Serial.begin(USBBaud); // USB Connection
  Serial1.begin(WIFLYBaud); // WIFLY Connection
  delay(2000); // Allow time for serial monitor to be opened
  Serial.println("Starting");
  Serial.print("Free memory: ");
  Serial.println(wifly.getFreeMemory(),DEC);

  elevator.attach(elevpin);
  rudder.attach(ruddpin);
  leftMO.attach(leftpin);
  rigtMO.attach(rigtpin);

  leftMO.write(50);
  rigtMO.write(50);

  if (!wifly.begin(&Serial1, &Serial)) {
    Serial.println("Failed to start wifly");
    terminal();
  }

  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
  }

  gyro.enableDefault();

  if (wifly.getFlushTimeout() != 10) {
    Serial.println("Restoring flush timeout to 10msecs");
    wifly.setFlushTimeout(10);
    wifly.save();
    wifly.reboot();
  }

  /* Setup the WiFly to connect to a wifi network */
  Serial.println("Joining network");
  wifly.setSSID(mySSID);
  //wifly.setPassphrase(myPassword);
  //wifly.enableDHCP();
  wifly.disableDHCP();
  wifly.setIP("192.168.1.108");  


  if (wifly.join()) {

    Serial.println("Joined wifi network");

  } 
  else {

    Serial.println("Failed to join wifi network");
    terminal();

  }

  wifly.setIP("192.168.1.108");
  wifly.setPort(2000);
  wifly.setNetmask("255.255.255.0");
  wifly.setGateway("192.168.1.1");

  /* Setup for UDP packets, sent automatically */
  wifly.setIpProtocol(WIFLY_PROTOCOL_UDP);
  wifly.setHost("192.168.1.1", 2000);	// Send UDP packet to this server and port
  wifly.setDeviceID("PHEONIX");

  char buf[32];

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

uint32_t e=0; //Elevator
uint32_t r=0; //Rudder

uint32_t command_mode = 0;

static char commandBuff[30];
uint32_t buffPos = 0;
//TinyGPSPlus gps;

void loop()
{

  /*
  //Read GPS
   if(ss.available() > 0){
   gps.encode(ss.read());
   }
   */

  //Read Input from wifly (Cached Data)
  if (Serial1.available() > 0) {
    //This function will also 
    updateControls(Serial1.read());
  }  

  //Send stuff to controller
  if (millis() > (lastUpdate + UPDATE_RATE))
  {
    //Serial.print("Sending update...");
    //gyro.read();
    sendData();
    //Serial.println("SUCCESS!");
    lastUpdate = millis();
  }

}

void updateControls(int readch) {

  //Serial.print(readch);

  if(readch == '&') {

    readCommand(commandBuff); // Read from command buffer
    memset(commandBuff, 0 , sizeof(commandBuff)); // Clear command buffer

  } 
  else {
    commandBuff[buffPos] = char(readch);
    if(buffPos == 30) {
      buffPos = 0;
    } 
    else 
      buffPos++
    }
  }
}

int readCommand(char* input) {
  char* command = strtok(input, "&");
  while (command != 0)
  {
    // Split the command in 2 values
    char* separator = strchr(command, ':');
    if (separator != 0)
    {
      // Actually split the string in 2: replace ':' with 0
      *separator = 0;
      ++separator;
      int position = atoi(separator) + 30;
      int test = atoi(command);
      setPositions(test, position);
      // Do something with servoId and position
    }
    // Find the next command in input string
    command = strtok(0, "&");
  }
  return 1;
}

int setPositions(int command, int position) {

  Serial.print(command);
  Serial.print(":");
  Serial.print(position);
  Serial.println("&");
  switch(command) {
  case 0:
    //Need to sort this one out
    break;
  case 1:
    rudder.write(position);
    break;
  case 2:
    elevator.write(position);
    break;
  case 3:
    if(position > 50) {
      leftMO.write(position);
      rigtMO.write(position);
    } 
    else {
      leftMO.write(50);
      rigtMO.write(50);
    }
    break;
  default:
    //Things
    break;
  }
  return 1;
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