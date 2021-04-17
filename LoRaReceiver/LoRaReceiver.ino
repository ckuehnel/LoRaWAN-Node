/* 
 * File: LoRaReceiver.ino
 * Basic test program, send date at the BAND you set.
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * www.heltec.cn
 * 
 * this project also realess in GitHub:
 * https://raw.githubusercontent.com/HelTecAutomation/Heltec_ESP32/master/examples/LoRa/LoRaReceiver/LoRaReceiver.ino
 */

#include "heltec.h"

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6
void setup() 
{
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
}

void loop() 
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) 
    {
      Serial.print((char)LoRa.read());
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
