/*
 Name:		Beweging_Visualisatie.ino
 Created:	1/9/2022 9:18:54 PM
 Author:	lesley wagner
 
 Description:	This application takes data from a the MPU9250 9 DOF orientation sensor,
 estimates orientation and position based on those measurements using the madgwick algorithm and triangular integration and 
 sends the orientation and position data to a computer via WiFi where these are visualized.
*/

#include <WiFiUdp.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiServerSecure.h>
#include <WiFiServer.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <BearSSLHelpers.h>
#include <ArduinoWiFiServer.h>
#include <Wire.h>
#include <list>
#include "quaternionFilters.h"
#include "MPU9250.h"
#include "MotionSensor.h"
#include "Motion.h"
#include "Components.h"
#include "WiFiModule.h"

#define AHRS true         // Set to false for basic data read
#define SerialDebug false  // Set to true to get Serial output for debugging
#define Wifi true
#define BUFFER_SIZE 100 // buffer size for the position and orientation data samples
#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0   // Use either this line or the next to select which I2C address your device is using

// Pin definitions
int intPin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins
int myLed = 13;  // Set up pin 13 led for toggling

MotionSensor myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);
Motion motionLib;

// last 5 low pass filtered and scaled acceleration samples
// the front of the list holds the oldest sample
std::list<Components> filteredAccs(5);
Components currentAcc;
Components currentAccLP; // current acceleration after low pass filter
Components scaledAcc; // current scaled acceleration
Components currentAccHP; // current acceleration after high pass filter
Components currentVel;
Components currentVelLP; // current angular velocity after low pass filter
Components scaledVel; // current scaled angular velocity
Components currentMag;
Components currentMagLP; // current magnetic force after low pass filter
Components scaledMag; // current scaled magnetic force

// buffers of orientation and position data that haven't been send yet
// the front of the lists hold the oldest samples
Components orientations[BUFFER_SIZE];
Components positions[BUFFER_SIZE];
Components currentPosition;
Components currentOrientation;
int motIndex = 0; // index of the last orientation and position data

char synchronizationByte = '_';
uint8_t dataPacket[17];
int packetSize = 17;


IPAddress local_IP(10, 10, 10, 1);
IPAddress gateway(10, 10, 10, 1);
IPAddress subnet(255, 255, 255, 0);
String hostname = "ESP8266";
const unsigned int localUdpPort = 4210;
int readErrors = 0; // number of errors on reading a wifi message

WiFiModule wifi(local_IP, gateway, subnet, hostname);

float example = 10;
long start;
int count = 0;
long t0;
long t1;

void setup() {
    // Wire.begin();
    // TWBR = 12;  // 400 kbit/sec I2C speed
    Serial.begin(9600);

    while (!Serial) {};

    currentPosition = { 0, 0, 0 };
    currentOrientation = { 0, 0, 0 };

    // Set up the interrupt pin, its set as active high, push-pull
    pinMode(intPin, INPUT);
    digitalWrite(intPin, LOW);
    pinMode(myLed, OUTPUT);
    digitalWrite(myLed, HIGH);

    // Read the WHO_AM_I register, this is a good test of communication
    byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    Serial.print(F("MPU9250 I AM 0x"));
    Serial.print(c, HEX);
    Serial.print(F(" I should be 0x"));
    Serial.println(0x71, HEX);

    if (c == 0x71) // WHO_AM_I should always be 0x71
    {
        // Serial.println(F("MPU9250 is online..."));

        // Start by performing self test and reporting values
        myIMU.MPU9250SelfTest(myIMU.selfTest);
        /*Serial.print(F("x-axis self test: acceleration trim within : "));
        Serial.print(myIMU.selfTest[0], 1); Serial.println("% of factory value");
        Serial.print(F("y-axis self test: acceleration trim within : "));
        Serial.print(myIMU.selfTest[1], 1); Serial.println("% of factory value");
        Serial.print(F("z-axis self test: acceleration trim within : "));
        Serial.print(myIMU.selfTest[2], 1); Serial.println("% of factory value");
        Serial.print(F("x-axis self test: gyration trim within : "));
        Serial.print(myIMU.selfTest[3], 1); Serial.println("% of factory value");
        Serial.print(F("y-axis self test: gyration trim within : "));
        Serial.print(myIMU.selfTest[4], 1); Serial.println("% of factory value");
        Serial.print(F("z-axis self test: gyration trim within : "));
        Serial.print(myIMU.selfTest[5], 1); Serial.println("% of factory value");*/

        // Calibrate gyro and accelerometers, load biases in bias registers
        myIMU.calibrateMPU9250();

        myIMU.initMPU9250();
        // Initialize device for active mode read of acclerometer, gyroscope, and
        // temperature
        // Serial.println("MPU9250 initialized for active data mode....");

        // Read the WHO_AM_I register of the magnetometer, this is a good test of
        // communication
        byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
        /*Serial.print("AK8963 ");
        Serial.print("I AM 0x");
        Serial.print(d, HEX);
        Serial.print(" I should be 0x");
        Serial.println(0x48, HEX);*/

        if (d != 0x48) {
            // Communication failed, stop here
            /*Serial.println(F("Communication failed, abort!"));
            Serial.flush();*/
            abort();
        }

        // Get magnetometer calibration from AK8963 ROM
        myIMU.initAK8963();
        // calibrate AK8963 by moving the MPU9250 in a figure 8
        myIMU.magCalMPU9250();
        // Initialize device for active mode read of magnetometer
        // Serial.println("AK8963 initialized for active data mode....");

        if (SerialDebug) {
            //  Serial.println("Calibration values: ");
            Serial.print("X-Axis factory sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[0], 2);
            Serial.print("Y-Axis factory sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[1], 2);
            Serial.print("Z-Axis factory sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[2], 2);
        }

        // Get sensor resolutions, only need to do this once
        myIMU.getAres();
        myIMU.getGres();
        myIMU.getMres();

        // The next call delays for 4 seconds, and then records about 15 seconds of
        // data to calculate bias and scale.
    //    myIMU.magCalMPU9250(myIMU.magBias, myIMU.magScale);
        /*Serial.println("AK8963 mag biases (mG)");
        Serial.println(myIMU.magBias[0]);
        Serial.println(myIMU.magBias[1]);
        Serial.println(myIMU.magBias[2]);*/

        /*Serial.println("AK8963 mag scale (mG)");
        Serial.println(myIMU.magScale[0]);
        Serial.println(myIMU.magScale[1]);
        Serial.println(myIMU.magScale[2]);*/
        //    delay(2000); // Add delay to see results before serial spew of data

        if (SerialDebug) {
            Serial.println("Magnetometer:");
            Serial.print("X-Axis sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[0], 2);
            Serial.print("Y-Axis sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[1], 2);
            Serial.print("Z-Axis sensitivity adjustment value ");
            Serial.println(myIMU.factoryMagCalibration[2], 2);
        }
    } // if (c == 0x71)
    else {
        Serial.print("Could not connect to MPU9250: 0x");
        Serial.println(c, HEX);

        // Communication failed, stop here
        Serial.println(F("Communication failed, abort!"));
        Serial.flush();
        abort();
    }
    if (Wifi) {
        if (wifi.listen()) {
            Serial.println("Connection established.");
        }
        wifi.begin(localUdpPort);
        while (!wifi.receive()) {};
    }
}

void loop() {
    // get sensor data if it is available
    myIMU.updateData(); // must be called before getting the data
    currentAcc = myIMU.getAcc();
    currentVel = myIMU.getVel();
    currentMag = myIMU.getMag();

    //// filter out noise from the data
    //// currentAccLP, currentVelLP, currentMagLP in the argument lists are the previous filtered samples
    //currentAccLP = myIMU.accLPF(currentAcc, currentAccLP);
    //currentVelLP = myIMU.velLPF(currentVel, currentVelLP);
    //currentMagLP = myIMU.magLPF(currentMag, currentMagLP);

    //// scale data after noise filtering
    //scaledAcc = myIMU.scaleAcc(currentAccLP);
    //scaledVel = myIMU.scaleVel(currentVelLP);
    //scaledMag = myIMU.scaleMag(currentMagLP);
    filteredAccs.pop_front();
    filteredAccs.push_back(currentAcc);

    if (SerialDebug) {
        /*Serial.print("ax = ");  Serial.print((int)1000 * myIMU.ax);
        Serial.print(" ay = "); Serial.print((int)1000 * myIMU.ay);
        Serial.print(" az = "); Serial.print((int)1000 * myIMU.az);
        Serial.println(" mg");*/

        /*Serial.print("gx = ");  Serial.print(myIMU.gx, 2);
        Serial.print(" gy = "); Serial.print(myIMU.gy, 2);
        Serial.print(" gz = "); Serial.print(myIMU.gz, 2);
        Serial.println(" deg/s");*/

        /*Serial.print("mx = ");  Serial.print((int)myIMU.mx);
        Serial.print(" my = "); Serial.print((int)myIMU.my);
        Serial.print(" mz = "); Serial.print((int)myIMU.mz);
        Serial.println(" mG");

        Serial.print("q0 = ");  Serial.print(*getQ());
        Serial.print(" qx = "); Serial.print(*(getQ() + 1));
        Serial.print(" qy = "); Serial.print(*(getQ() + 2));
        Serial.print(" qz = "); Serial.println(*(getQ() + 3));*/
    }

    currentAccHP = myIMU.accHPF(filteredAccs);

    currentOrientation = motionLib.calcOrientation(currentAcc, currentVel, currentMag, myIMU.deltat);
    // current position in the argument list is actually the previous position
    currentPosition = motionLib.calcPosition(currentAccHP, currentOrientation, currentPosition, myIMU.deltat);

    orientations[motIndex] = currentOrientation;
    positions[motIndex] = currentAccHP;
    if (Wifi)
    {
        motIndex++;
        if (motIndex == 5) {
            wifi.send(orientations, positions, 5);
            motIndex = 0;
        }

        // Verify that the client is connected every second
        if (millis() - myIMU.count > 1000) {
            if (!wifi.receive()) {
                readErrors++;
            }
            else {
                readErrors = 0;
            }
            // if there are 3 read errors in a row, restart the connection and wait for a message.
            if (readErrors == 3) {
                wifi.close();
                wifi.begin(localUdpPort);

                while (!wifi.receive()) {};

                readErrors = 0;
            }

            myIMU.count = millis();
        }
    }

    /*Components dummyOrientation = { 5.0, 5.0, 5.0 };
    Components dummyPosition = { 1.0, 2.0, 3.5 };*/

    if (SerialDebug) {
        // count++;
        /*Serial.print("Yaw, Pitch, Roll: ");
        Serial.print(myIMU.yaw, 2);
        Serial.print(", ");
        Serial.print(myIMU.pitch, 2);
        Serial.print(", ");
        Serial.println(myIMU.roll, 2);*/
        /*Serial.print("Position x, y, z: ");
        Serial.print(currentPosition.x, 4);
        Serial.print(", ");
        Serial.print(currentPosition.y, 4);
        Serial.print(", ");
        Serial.println(currentPosition.z, 4);*/
        /*Serial.print("period = ");
        Serial.println((millis() - start) / (float)count, 2);*/
        // Serial.println(" ms");
    }
}