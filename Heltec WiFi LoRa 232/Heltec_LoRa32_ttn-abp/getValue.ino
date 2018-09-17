// This function read a DHT11 sensor

void getValue()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int16_t temp = (int) temperature *10;
  uint8_t  humi = (int) humidity; 

  char buffer[5];
  String s = dtostrf(temperature, 3, 1, buffer);
  display.clear();
  display.drawString(0, 0, "Temp = ");
  display.drawString(72, 0, s);
  display.drawString(104, 0, " °C");

  s = dtostrf(humidity, 3, 0, buffer);
  display.drawString(0, 16, "Hum  = ");
  display.drawString(72, 16, s);
  display.drawString(104, 16, " %");

  display.display();

  mydata[0] = highByte(temp);
  mydata[1] = lowByte(temp);
  mydata[2] = humi;

  if (DEBUG)
  {
    Serial.print("Measured temperature = ");
    Serial.print(temperature, 1); 
    Serial.println(" *C"); 
    Serial.print("Measured humidity = ");
    Serial.print(humidity, 0); 
    Serial.println(" % rH");   
  }
}

