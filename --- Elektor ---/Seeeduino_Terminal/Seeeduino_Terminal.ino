/* 
 *  File: Seeeduino_Terminal.ino
 *  
 *  Send AT commandos to the Seeeduino LoRaWAN module
 *  https://wiki.seeedstudio.com/LoRa_LoRaWan_Gateway_Kit/
 *  
 */
void setup()
{
   Serial1.begin(9600);
   SerialUSB.begin(115200);
 }
 
void loop()
{
  while(Serial1.available())
  {
     SerialUSB.write(Serial1.read());
  }
  while(SerialUSB.available())
  {
     Serial1.write(SerialUSB.read());
  }
}
