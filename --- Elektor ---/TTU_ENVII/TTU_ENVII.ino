/*
 * File: TTU_ENVII.ino
 * 
 * Sending data sampled by ENV.II Unit w/ The Things Uno board to TTN
 * based on example SendOTAA
 * 
 * 2021-06-25 Claus Kühnel info@ckuehnel.ch
 */
#include <TheThingsNetwork.h>

// Set your AppEUI and AppKey
const char *appEui = "70B3D57ED0007939";
const char *appKey = "7828A5B0225E6D6EBE1C23ABCC44E2E7";

#define DEBUG 1   // set to 0 to avoid display of measuring results on serial monitor

float sht31Temperature, sht31Humidity;
float bmp280Temperature, bmp280Pressure;

const long CYCLE = 5 * 60000; // Transmission cycle 5 minutes
unsigned long previousMillis = 0;

#define loraSerial Serial1

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

TheThingsNetwork ttn(loraSerial, Serial, freqPlan);

void setup()
{
  loraSerial.begin(57600);
  Serial.begin(9600);

  while(!Serial);  // wait for serial monitor

  Serial.println("\nSensornode w/ M5Stack ENV.II Sensor"); // Change this if you want
  Serial.println("SHT31 @ 0x44 & BMP280 @ 0x76");
  Serial.println("Initialize sensors...");
  initSensor(); // initialize ENV.II sensor
  Serial.println("Initialization done.");
  pinMode(LED_BUILTIN, OUTPUT);  

  Serial.println("-- STATUS");
  ttn.showStatus();

  Serial.println("-- JOIN");
  ttn.join(appEui, appKey);
}

void loop()
{
  Serial.println("-- LOOP");

  getValues(); // get measuring results of ENV.II sensor

  // prepare the measuring results for use in payload
  // you can enhance payload w/ BMP280 data, if you want
  int16_t temp = (int16_t) (sht31Temperature * 100. + .5);
  uint8_t tempHi = highByte(temp);
  uint8_t tempLo = lowByte(temp);

  int16_t humi = (int16_t) (sht31Humidity * 10. + .5);
  uint8_t humiHi = highByte(humi);
  uint8_t humiLo = lowByte(humi);

  // Prepare payload of 4 byte conatining measuring results
  byte payload[4];
  payload[0] = tempHi;
  payload[1] = tempLo;
  payload[2] = humiHi;
  payload[3] = humiLo;

  digitalWrite(LED_BUILTIN, HIGH);

  // Send it off
  ttn.sendBytes(payload, sizeof(payload));

  digitalWrite(LED_BUILTIN, LOW);

  previousMillis = millis();
  while(millis() - previousMillis < CYCLE);  // wait CYCLE milliseconds
}
