/*
 * File: DS2401_DevEUI.ino
 * Read DevEUI from DS2401 w/ any Arduino via GPIO
 * Used Library: DS2401 by https://github.com/Ideetron/Nexus_LoRaWAN
 * 
 * 2021-07-13 Claus Kühnel info@ckuehnel.ch
 */

#include "DS2401.h"     // set pin for DS2401 there
#define Serial SerialUSB // for Seeeduino XIAO

uint8_t DevEUI[8];      //  Device EUI. Unique number to identify the Mote with on the back-end.

void setup() 
{
  Serial.begin(115200);
  delay(2000);          // wait for Serial Monitor
  Serial.println("\nDetermine a DevEUI from DS2401 Silicon Serial Number");
  Serial.print("Reading DS2401...");
  DS_Read(&DevEUI[0]);
  Serial.println("done.");
  printStringAndHex("DS2401 DEV EUI: ", &DevEUI[0], 8);
}

void loop() {}

void printStringAndHex(const char *String, uint8_t *data, uint8_t n)
{
  uint8_t i;
  Serial.print(String);
  Serial.flush();
  Serial.print(n, DEC);
  Serial.print(" bytes; ");
  
  for( i = 0 ; i < n ; i++)      // Print the data as a hexadecimal string
  {
    // Print single nibbles, since the Hexadecimal format printed by the Serial.Print function does not print leading zeroes.
    Serial.print((unsigned char) ((data[i] & 0xF0) >> 4), HEX); // Print MSB first
    Serial.print((unsigned char) ((data[i] & 0x0F) >> 0), HEX); // Print LSB second
    Serial.print(' ');
    Serial.flush();
  }
  Serial.println();
}
