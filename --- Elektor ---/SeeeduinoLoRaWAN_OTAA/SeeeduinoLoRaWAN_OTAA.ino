/*
 * Fie: SeeeduinoLoRaWAN_OTAA
 * 
 * Sending CayenneLPP coded messages to TTS (CE)
 * 
 * based on https://github.com/brady-aiello/Seeeduino_LoRaWAN_for_hybrid_gateways
 * 
 * 2021-07-29 Claus Kühnel info@ckuehnel.ch
 */

#include<LoRaWan.h>
#include <CayenneLPP.h>
#include "arduino_secrets.h"

#define DEBUG 1

#define FREQ_RX_WNDW_SCND_EU  869.525

const float EU_hybrid_channels[8] = {868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9}; //rx 869.525

#define DOWNLINK_DATA_RATE_EU    DR5
#define UPLINK_DATA_RATE_MAX_EU  DR5
#define MAX_EIRP_NDX_EU  2

//The min uplink data rate for all countries / plans is DR0
#define UPLINK_DATA_RATE_MIN DR0

#define DEFAULT_RESPONSE_TIMEOUT 5
unsigned char frame_counter = 1;
int loopcount = 0;
char buffer[256];

float sht31Temperature, sht31Humidity;
float bmp280Temperature, bmp280Pressure;

CayenneLPP lpp(51);  // maximum payload

void setup(void)
{
    SerialUSB.begin(115200);
    //while(!SerialUSB); 
    delay(2000);    
    
    lora.init();
    lora.setDeviceReset();
    initSensor();
  
    memset(buffer, 0, 256);
    lora.getVersion(buffer, 256, 1);
    SerialUSB.print(buffer); 
       
    memset(buffer, 0, 256);
    lora.getId(buffer, 256, 1);
    SerialUSB.print(buffer);

    lora.setId(NULL, DEV_EUI, APP_EUI);
    lora.setKey(NULL, NULL, APP_KEY);
    
    lora.setDeciveMode(LWOTAA);
    lora.setDataRate(DR5, EU868);
    //lora.setAdaptiveDataRate(true); 
    setHybridForTTN(EU_hybrid_channels);
    
    lora.setReceiceWindowFirst(0, EU_hybrid_channels[0]);
    lora.setReceiceWindowSecond(FREQ_RX_WNDW_SCND_EU, DOWNLINK_DATA_RATE_EU);

    lora.setDutyCycle(false);
    lora.setJoinDutyCycle(false);
    lora.setPower(MAX_EIRP_NDX_EU);
}

void setHybridForTTN(const float* channels)
{
  for(int i = 0; i < 8; i++)
  {
    if(channels[i] != 0)
    {
      lora.setChannel(i, channels[i], UPLINK_DATA_RATE_MIN, UPLINK_DATA_RATE_MAX_EU);   
    }
  }
}

void loop(void)
{
  prepareMessage();
  
  while(!lora.setOTAAJoin(JOIN));
  lora.transferPacket(lpp.getBuffer(), lpp.getSize(), DEFAULT_RESPONSE_TIMEOUT);
  lora.loraDebug();
  delay(5000);
}

void prepareMessage()
{
  getValues(); 
  
  lpp.reset();
  lpp.addTemperature(1, sht31Temperature);
  lpp.addRelativeHumidity(2, sht31Humidity);
  lpp.addTemperature(3, bmp280Temperature);
  lpp.addBarometricPressure(4, bmp280Pressure);

  if (DEBUG)
  { 
    Serial.print("Payload length =  ");
    Serial.println(lpp.getSize());

    uint8_t *payload = lpp.getBuffer();

    for (uint8_t i = 0; i < lpp.getSize(); i++)
    {
      Serial.print("0x");
      Serial.print(payload[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}
