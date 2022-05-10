#include "WiFiModule.h"

bool WiFiModule::listen() {
	WiFi.softAP("ESPsoftAP_01", "proxidiagnost");
	while (WiFi.softAPgetStationNum() == 0) {
		delay(500);
		Serial.print(".");
	}
	return true;
}

void WiFiModule::begin(int udpPort) {
	udp.begin(udpPort);
}

void WiFiModule::close() {
    udp.stop();
}

bool WiFiModule::send(Components* orientations, Components* positions, int num_samples) {
    // the message is (brackets indicate a data value): [_Px{position.x}y{position.y}z{position.y}Ox{orientation.x}y{Orientation.y}z{Orientation.z}
    uint8_t dataPacket[1000];
    for (int i = 0; i < num_samples; i++) {
        int offset = i * packetSize;
        dataPacket[0 + offset] = synchronizationByte;
        dataPacket[1 + offset] = 'P';
        dataPacket[2 + offset] = 'x';
        dataPacket[3 + offset] = *(uint8_t*)&positions[i].x;
        dataPacket[4 + offset] = *((uint8_t*)&positions[i].x + 1);
        dataPacket[5 + offset] = *((uint8_t*)&positions[i].x + 2);
        dataPacket[6 + offset] = *((uint8_t*)&positions[i].x + 3);
        dataPacket[7 + offset] = 'y';
        dataPacket[8 + offset] = *(uint8_t*)&positions[i].y;
        dataPacket[9 + offset] = *((uint8_t*)&positions[i].y + 1);
        dataPacket[10 + offset] = *((uint8_t*)&positions[i].y + 2);
        dataPacket[11 + offset] = *((uint8_t*)&positions[i].y + 3);
        dataPacket[12 + offset] = 'z';
        dataPacket[13 + offset] = *(uint8_t*)&positions[i].z;
        dataPacket[14 + offset] = *((uint8_t*)&positions[i].z + 1);
        dataPacket[15 + offset] = *((uint8_t*)&positions[i].z + 2);
        dataPacket[16 + offset] = *((uint8_t*)&positions[i].z + 3);
        dataPacket[17 + offset] = 'O';
        dataPacket[18 + offset] = 'x';
        dataPacket[19 + offset] = *(uint8_t*)&orientations[i].x;
        dataPacket[20 + offset] = *((uint8_t*)&orientations[i].x + 1);
        dataPacket[21 + offset] = *((uint8_t*)&orientations[i].x + 2);
        dataPacket[22 + offset] = *((uint8_t*)&orientations[i].x + 3);
        dataPacket[23 + offset] = 'y';
        dataPacket[24 + offset] = *(uint8_t*)&orientations[i].y;
        dataPacket[25 + offset] = *((uint8_t*)&orientations[i].y + 1);
        dataPacket[26 + offset] = *((uint8_t*)&orientations[i].y + 2);
        dataPacket[27 + offset] = *((uint8_t*)&orientations[i].y + 3);
        dataPacket[28 + offset] = 'z';
        dataPacket[29 + offset] = *(uint8_t*)&orientations[i].z;
        dataPacket[30 + offset] = *((uint8_t*)&orientations[i].z + 1);
        dataPacket[31 + offset] = *((uint8_t*)&orientations[i].z + 2);
        dataPacket[32 + offset] = *((uint8_t*)&orientations[i].z + 3);
    }
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(dataPacket, packetSize*num_samples);
    return udp.endPacket();
}

bool WiFiModule::send(Components& orientation, Components& position) {
    // the message is (brackets indicate a data value): [_Px{position.x}y{position.y}z{position.y}Ox{orientation.x}y{Orientation.y}z{Orientation.z}
    uint8_t dataPacket[33];
    dataPacket[0] = synchronizationByte;
    dataPacket[1] = 'P';
    dataPacket[2] = 'x';
    dataPacket[3] = *(uint8_t*)&position.x;
    dataPacket[4] = *((uint8_t*)&position.x + 1);
    dataPacket[5] = *((uint8_t*)&position.x + 2);
    dataPacket[6] = *((uint8_t*)&position.x + 3);
    dataPacket[7] = 'y';
    dataPacket[8] = *(uint8_t*)&position.y;
    dataPacket[9] = *((uint8_t*)&position.y + 1);
    dataPacket[10] = *((uint8_t*)&position.y + 2);
    dataPacket[11] = *((uint8_t*)&position.y + 3);
    dataPacket[12] = 'z';
    dataPacket[13] = *(uint8_t*)&position.z;
    dataPacket[14] = *((uint8_t*)&position.z + 1);
    dataPacket[15] = *((uint8_t*)&position.z + 2);
    dataPacket[16] = *((uint8_t*)&position.z + 3);
    dataPacket[17] = 'O';
    dataPacket[18] = 'x';
    dataPacket[19] = *(uint8_t*)&orientation.x;
    dataPacket[20] = *((uint8_t*)&orientation.x + 1);
    dataPacket[21] = *((uint8_t*)&orientation.x + 2);
    dataPacket[22] = *((uint8_t*)&orientation.x + 3);
    dataPacket[23] = 'y';
    dataPacket[24] = *(uint8_t*)&orientation.y;
    dataPacket[25] = *((uint8_t*)&orientation.y + 1);
    dataPacket[26] = *((uint8_t*)&orientation.y + 2);
    dataPacket[27] = *((uint8_t*)&orientation.y + 3);
    dataPacket[28] = 'z';
    dataPacket[29] = *(uint8_t*)&orientation.z;
    dataPacket[30] = *((uint8_t*)&orientation.z + 1);
    dataPacket[31] = *((uint8_t*)&orientation.z + 2);
    dataPacket[32] = *((uint8_t*)&orientation.z + 3);
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(dataPacket, packetSize);
    return udp.endPacket();
}

bool WiFiModule::receive() {
    if (udp.parsePacket() == 9) {
        char currentMessage[10];
        udp.readBytes(currentMessage, receiveLength);
        currentMessage[9] = 0;
        if (!strcmp(currentMessage, receiveMessage)) {
            return true;
        }
    }

    return false;
}
