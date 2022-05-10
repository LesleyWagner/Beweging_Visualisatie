#pragma once
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Components.h"
#include "OrientationPacket.h"
#include <string>

class WiFiModule {
public:
	WiFiModule(IPAddress& local_IP, IPAddress& gateway, IPAddress& subnet, String& hostname) {
		WiFi.mode(WIFI_AP_STA);
		WiFi.hostname(hostname.c_str());
		WiFi.softAPConfig(local_IP, gateway, subnet);
	}

	/*
	* Starts WiFi access point and listens for an incoming connection.
	* Returns true once a connection has been established.
	* Otherwise continues indefinitely.
	*/
	bool listen();

	/*
	* Starts UDP communication on port 'udpPort'.
	*/
	void begin(int udpPort);

	/*
	* Close UDP connection.
	*/
	void close();

	/*
	* Sends sensor data via UDP communication.
	* Returns true if the message was succesfully sent, false otherwise.
	*/
	bool send(Components& orientation, Components& position);

	/*
	* Sends 'num_samples' number of samples of sensor data via UDP communication.
	* 'orientation' and 'position' must be arrays of size 'num_samples'.
	* Returns true if the message was succesfully sent, false otherwise.
	*/
	bool send(Components* orientation, Components* position, int num_samples);

	/*
	* Receives data from client via UDP communication.
	* Returns true if the message was succesfully received, false otherwise.
	*/
	bool receive();

private:
	WiFiUDP udp;
	char synchronizationByte = '_';
	const int packetSize = 33;
	char receiveMessage[10] = "connected";
	const int receiveLength = 9; // length of the receive message
};

