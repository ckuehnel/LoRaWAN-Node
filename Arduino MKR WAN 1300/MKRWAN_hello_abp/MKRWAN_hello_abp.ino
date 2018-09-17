
// Sending a string to the LoRaWAN server
// based on
// https://github.com/gonzalocasas/arduino-mkr-wan-1300
// Modifications: Claus Kühnel info@ckuehnel.ch 2018-09-12

#include <MKRWAN.h>
#include "arduino_secrets.h" 

#define PERIOD 60000

// Select your region (AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID)
_lora_band region = EU868;

LoRaModem modem(Serial1);

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  if (!modem.begin(region)) 
  {
    Serial.println("Failed to start module");
    while (1) {}
  }
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinABP(devAddr, nwkSKey, appSKey);
  if (!connected) 
  {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }
  Serial.println("Successfully joined the network!");

  Serial.println("Enabling ADR and setting low spreading factor");
  modem.setADR(true);
  modem.dataRate(5);
}

void loop() 
{
  modem.beginPacket();
  modem.print("HeLoRaWAN");
  
  int err = modem.endPacket(false);

  if (err > 0) Serial.println("Package sent.");
  else         Serial.println("Error");

  delay(PERIOD);
}
