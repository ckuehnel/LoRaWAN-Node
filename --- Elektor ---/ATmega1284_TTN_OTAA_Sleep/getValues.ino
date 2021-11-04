/*
 * Get values from SHT2x & LM75 sensor and battery voltage
 */

void getValues()
{
  // Vext einschalten
  digitalWrite(VEXT, HIGH);
  delay(50);
  
  // Read SHT2x values
  float SHT2x_temp = SHT2x.GetTemperature();
  extmp = (int16_t) (SHT2x_temp * 100 + 0.5);      // external Temperature as word
  float SHT2x_humi = SHT2x.GetHumidity();
  exhum = (uint16_t) (SHT2x_humi * 10 + 0.5);      // external Humidity as word
  #ifdef DEBUG
    Serial.print("SHT2x Temperature = "); Serial.print(SHT2x_temp); Serial.println(" °C");
    Serial.println(extmp,HEX);
    Serial.print("SHT2x Humidity    = "); Serial.print(SHT2x_humi); Serial.println(" %RH");
    Serial.println(exhum,HEX);
  #endif

  // Read LM75 value
  float LM75_temp  = LM75.readTemperatureC();
  itmp = (int16_t) (LM75_temp * 100 + 0.5);      // internal Temperature as word
  #ifdef DEBUG
    Serial.print("LM75 Temperature  = "); Serial.print(LM75_temp); Serial.println(" °C");
    Serial.println(itmp,HEX);
  #endif

  // Vext ausschalten
  digitalWrite(VEXT, LOW);

  // Read battery voltage
  LadA = digitalRead(CHRG);  // Liest den Inputpin
  unsigned int sensorValue = analogRead(A7);
  float voltage = sensorValue * (3.3 / 1024.0)*2;
  volt = (uint16_t) (voltage * 1000 + 0.5);  // Battery voltage in mV as word
  volt = volt | 0x8000;
  #ifdef DEBUG  
    Serial.print("Battery Voltage   = "); Serial.print(voltage); Serial.println(" V");
    Serial.print("Loading: ");
    if (LadA) Serial.println("No"); 
    else      Serial.println("Yes");
    Serial.println(volt,HEX);
  #endif
}
