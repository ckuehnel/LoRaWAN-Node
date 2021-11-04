/*
 * File: mkrwan_01_get_deveui.ino
 * 
 * Get DevEUI of ARDUINO MKR WAN 1300 board
 * Source: https://github.com/gonzalocasas/arduino-mkr-wan-1300/tree/master/mkrwan_01_get_deveui
 * 
 */

#include <MKRWAN.h>

// Select your region (AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID)
_lora_band region = EU868;

LoRaModem modem(Serial1);

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nGet DevEUI from Murata CMWX1ZZABZ");
  if (!modem.begin(region)) 
  {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());
}

void loop() {}
