/*
 * File: XIAO_Terminal.ino
 * 
 * Controlling LoRaWAN868 Unit serial by AT Commands
 */
 
uint32_t baud;
uint32_t old_baud;

void setup() 
{
  SerialUSB.begin(115200);
  baud = SerialUSB.baud();
  old_baud = baud;
  Serial1.begin(baud);
  while (!Serial);
  while (!SerialUSB);
  Serial.println("XIAO Terminal");
  SerialUSB.print("Baudrate = ");
  SerialUSB.println(baud);
}
 
void loop() 
{
  baud = SerialUSB.baud();
  if (baud != old_baud) 
  {
    Serial1.begin(baud);
    while (!Serial);
    old_baud = baud;
    SerialUSB.println("Baudrate = ");
    SerialUSB.println(baud);
  }

  
  if (SerialUSB.available() > 0)
  {
    char c = SerialUSB.read();
    Serial1.write(c);
  }
  if (Serial1.available() > 0) {
    char c = Serial1.read();
    SerialUSB.write(c);
  }
}
