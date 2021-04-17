/****************************************************************************
**                                                                         **
** Name:        M5_Stack_CORE_TTN_SensorNode.ino                           **
** Author:      Achim Kern                                                 **
** Interpreter: Arduino IDE 1.8.13                                         **
** Licence:     Freeware                                                   **
** Function:    Main Program                                               **
**                                                                         **
** Notes:       based on idea from SEEED STUDIO and LCARS SmartHome        **
**                                                                         **
** History:                                                                **
**                                                                         **
** 1.00        - 24.01.2021 - initial release                              **
**                          - TTN joined                                   **
** 1.01        - 25.01.2021 - ENVII unit implemented                       **                
**                          - NeoPixel implemented                         **
**                          - battery level implemented                    **                          
**                          - TTN payload defined                          **                          
**                          - PIR Motion sensor - display on/off           **                          
**                          - sensor data on screen display                **
** 1.02        - 26.01.2020 - show jpg pictures on booting                 **                          
** 1.03        - 27.01.2021 - TTN access codes                             **
**                                                                         **
*****************************************************************************

/*
 * Application and Version
 */
   const char* application  = "M5STACK_CORE_TTN_SensorNode";
   const char* aktu_version = "1.03";

/*
 * M5 STACK TFT Display
 * 
 */
   // we need these libraries, defines, variables
   #include <M5Stack.h>
   // we use special fonts
   #include "Free_Fonts.h"
   // tft backlight - on/off button 1 - on/off after 1 minute
   bool tft_backlight = true; 
   // screen off counter 
   int tft_counter=0;
   // pictures on the sd card
   String iot_picture="";

/*
 * M5 STACK RGB NeoPixels
 * 
 */
   // we need these libraries, defines, variables   
   #include <Adafruit_NeoPixel.h>
   #define M5STACK_FIRE_NEO_NUM_LEDS 10
   #define M5STACK_FIRE_NEO_DATA_PIN 15
   Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);   

 /*
 * ENVII M5Stack Unit I2C Pressure & Temperature & Humidity Sensor
 * ---------------------------------------------------------------
 * ENV II is an environment sensor which can sense temperature, humidity and atmospheric pressure. 
 * It is built with SHT30 and BMP280 sensors and is programmed over I2C. 
 * SHT30 is a digital temperature and humidity sensor with high precision and low power consumption. 
 * BMP280 is an absolute barometric pressure sensor which is especially designed for mobile applications. 
 * It offers the highest flexibility to optimize the device regarding power consumption, resolution and filter performance.
 * 
 */
   // Enable/disable sensor measurements if you want to
   #define ENABLE_SENSOR_ENVII
   // Sensors enabled, but not found in the hardware will be ignored
   #ifdef ENABLE_SENSOR_ENVII
     #include <M5Stack.h>
     #include <Wire.h>
     #include "Adafruit_Sensor.h"
     #include <Adafruit_BMP280.h>
     #include "SHT3X.h"
     SHT3X sht30;
     Adafruit_BMP280 bme;
     float env2_tmp = 0.0;
     float env2_hum = 0.0;
     float env2_pressure = 0.0;
     /*-----------------------------------------*/
     /* Function void sensor_env2()             */
     /*                                         */
     /* TASK    : read out env2 sensor data     */
     /* UPDATE  : 04.11.2020                    */
     /*-----------------------------------------*/   
     void sensor_env2(void)
     {
       // read the sensor
       env2_pressure = bme.readPressure();
       env2_pressure = env2_pressure/100;
       if(sht30.get()==0)
       {
         env2_tmp = sht30.cTemp-1;
         env2_hum = sht30.humidity+5;
       }   
     }     
   #endif

/*
 * GROVE UNIT PIR Motion Sensor
 * ----------------------------
 * This sensor allows you to sense motion, usually human movement in its range. Simply connect it and program it, 
 * when anyone moves in its detecting range, the sensor will output HIGH on its SIG pin.
 *
 */
   // Enable/disable sensor measurements if you want to
   //#define ENABLE_SENSOR_PIR_MOTION
   // Sensors enabled, but not found in the hardware will be ignored
   #ifdef ENABLE_SENSOR_PIR_MOTION
     #define PIR_MOTION_SENSOR 36 
     bool MOTION=false;
   #endif

/*
 * connected sensor data place holders
 */
   // m5stack-core-01
   #define M5STACK 0
   int m5stack_temp  = 0;
   int m5stack_humi  = 0;
   int m5stack_press = 0;
   int m5stack_bat   = 0;

/*   
 * TTN OTAA access data 
 * we must use this in the setup() 
 */
  // DevEui=XXXXXXXXXXXXXXXX(For OTAA Mode)
  // DevEui="0000AC33A5C40A24";
   
  // AppEui=XXXXXXXXXXXXXXXX(For OTAA Mode)
  // always the same if equal devices
  // AppEui = "0000000000000000";
   
  // AppKey=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX(For OTAA Mode)
  // AppKey="00112233445566778899AABBCCDDEEFF"; 
   
   // ttn counter send frequence - used in loop()
   int TTNCounter=0;
      
/*
 * Generally, you should use "unsigned long" for variables that hold time
 * The value will quickly become too large for an int to store
 */
   // this timer is used to update tft display
   unsigned long previousMillis = 0;
   // every 1 minute
   //unsigned long interval = 60000;  
   // every 3 minutes
   // unsigned long interval = 180000;
   // every 30 seconds
   unsigned long interval = 30000;   
   unsigned long counter  = 0;

/*-------------------------------------------------------------------------------*/
/* Function void ATCommand(char cmd[],char date[], uint32_t timeout = 50)        */
/*                                                                               */
/* TASK    : send AT commands to the M5Stack COM.LoRaWAN Module                  */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void ATCommand(char cmd[],char date[], uint32_t timeout = 50)
{
  char buf[256] = {0};
  if(date == NULL)
  {
    sprintf(buf,"AT+%s",cmd);
  }
  else 
  {
    sprintf(buf,"AT+%s=%s",cmd,date); 
  }
  Serial2.write(buf);
  delay(200);
  ReceiveAT(timeout);
}

/*-------------------------------------------------------------------------------*/
/* Function bool ReceiveAT(uint32_t timeout)                                     */
/*                                                                               */
/* TASK    : receive AT msg's from the M5Stack COM.LoRaWAN Module                */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
bool ReceiveAT(uint32_t timeout)
{
  uint32_t nowtime = millis();
  while(millis() - nowtime < timeout){
    if (Serial2.available() !=0) {
      String str = Serial2.readString();
      if (str.indexOf("+OK") != -1 || str.indexOf("+ERROR") != -1) {
        Serial.println(str);
        return true;
      }else {
        Serial.println("[!] Syntax Error");
        break;
      }
    }
  }
  Serial.println("[!] Timeout");
  return false;
}

/*-------------------------------------------------------------------------------*/
/* Function void rgb_neopixel(String color)                                      */
/*                                                                               */
/* TASK    : show rgb neopixels (co2 ampel)                                      */
/* UPDATE  : 07.10.2020                                                          */
/*-------------------------------------------------------------------------------*/
void rgb_neopixel(int r,int g,int b)  
{
  // right side
  if (tft_backlight == true)
  {
    pixels.setPixelColor(0, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(1, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(2, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(3, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(4, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    // left side
    pixels.setPixelColor(5, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(6, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(7, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(8, pixels.Color(r,g,b));
    pixels.show(); delay(200);
    pixels.setPixelColor(9, pixels.Color(r,g,b));
    pixels.show(); delay(200);
  }  
}

/*-------------------------------------------------------------------------------*/
/* Function void rgb_neopixel_on(String color)                                   */
/*                                                                               */
/* TASK    : show rgb neopixels immediately                                      */
/* UPDATE  : 07.10.2020                                                          */
/*-------------------------------------------------------------------------------*/
void rgb_neopixel_on(int r,int g,int b)  
{
  // right side
  pixels.setPixelColor(0, pixels.Color(r,g,b));
  pixels.setPixelColor(1, pixels.Color(r,g,b));
  pixels.setPixelColor(2, pixels.Color(r,g,b));
  pixels.setPixelColor(3, pixels.Color(r,g,b));
  pixels.setPixelColor(4, pixels.Color(r,g,b));
  // left side
  pixels.setPixelColor(5, pixels.Color(r,g,b));
  pixels.setPixelColor(6, pixels.Color(r,g,b));
  pixels.setPixelColor(7, pixels.Color(r,g,b));
  pixels.setPixelColor(8, pixels.Color(r,g,b));
  pixels.setPixelColor(9, pixels.Color(r,g,b));
  pixels.show(); 
}

/*-------------------------------------------------------------------------------*/
/* Function void rgb_neopixel_off()                                              */
/*                                                                               */
/* TASK    : rgb neopixels off                                                   */
/* UPDATE  : 18.11.2020                                                          */
/*-------------------------------------------------------------------------------*/
void rgb_neopixel_off()  
{
  // right side
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.setPixelColor(1, pixels.Color(0,0,0));
  pixels.setPixelColor(2, pixels.Color(0,0,0));
  pixels.setPixelColor(3, pixels.Color(0,0,0));
  pixels.setPixelColor(4, pixels.Color(0,0,0));
  // left side
  pixels.setPixelColor(5, pixels.Color(0,0,0));
  pixels.setPixelColor(6, pixels.Color(0,0,0));
  pixels.setPixelColor(7, pixels.Color(0,0,0));
  pixels.setPixelColor(8, pixels.Color(0,0,0));
  pixels.setPixelColor(9, pixels.Color(0,0,0));
  pixels.show(); 
}

/*-------------------------------------------------------------------------------*/
/* Function void array_to_string(byte array[], unsigned int len, char buffer[])  */
/*                                                                               */
/* TASK    : build string out of payload data                                    */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_room_screen(String room_name, long int room_bg)     */
/*                                                                               */
/* TASK    : show tft display room screen                                        */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_room_screen(String room_name, long int room_bg) 
{
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.fillRect(0,0,320,50,room_bg);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth(room_name)) / 2, 32);
  M5.Lcd.print(room_name); 
  // drawing verticle line
  M5.Lcd.drawFastVLine(150,50,190,TFT_DARKGREEN);
  // drawing horizontal line
  M5.Lcd.drawFastHLine(0,140,320,TFT_DARKGREEN);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_status_bar(String room_name, long int room_bg)      */
/*                                                                               */
/* TASK    : show tft display status bar                                         */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_status_bar(String room_name, long int room_bg) 
{
  M5.Lcd.fillRect(0,0,320,50,room_bg);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth(room_name)) / 2, 32);
  M5.Lcd.print(room_name); 
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_sensor_temperature(void)                            */
/*                                                                               */
/* TASK    : show tft display sensor temperature                                 */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_sensor_temperature(int temperature) 
{
  // setting the temperature
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB9);
  M5.Lcd.drawString("Temperature",15,65);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(temperature,50,95);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.drawString("C",100,95);  
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_update_temperature(void)                            */
/*                                                                               */
/* TASK    : show tft display update temperature                                 */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_update_temperature(int temperature) 
{
  // setting the temperature
  M5.Lcd.fillRect(50,95,50,30,WHITE);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(temperature,50,95);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_sensor_humidity(int humidity)                       */
/*                                                                               */
/* TASK    : show tft display sensor humidity                                    */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_sensor_humidity(int humidity) 
{
  // setting the humidity
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB9);
  M5.Lcd.drawString("Humidity",30,160);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(humidity,50,190);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.drawString("%",100,190);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_update_humidity(void)                               */
/*                                                                               */
/* TASK    : show tft display update humidity                                    */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_update_humidity(int humidity) 
{
  // setting the humidity
  M5.Lcd.fillRect(50,190,50,30,TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(humidity,50,190);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_sensor_pressure(int pressure)                       */
/*                                                                               */
/* TASK    : show tft display sensor pressure                                    */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_sensor_pressure(int pressure) 
{
  // setting the pressure
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB9);
  M5.Lcd.drawString("Pressure",190,65);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(pressure,180,95);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.drawString("mBar",250,95);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_update_pressure(void)                               */
/*                                                                               */
/* TASK    : show tft display update pressure                                    */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_update_pressure(int pressure) 
{
  // setting the pressure
  M5.Lcd.fillRect(160,95,85,30,TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(pressure,160,95);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_sensor_battery(int battery)                         */
/*                                                                               */
/* TASK    : show tft display sensor battery                                     */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_sensor_battery(int battery) 
{ 
  // setting the battery power
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB9);
  M5.Lcd.drawString("Battery",200,160);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(battery,200,190);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.drawString("%",280,190);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_update_battery(int battery)                         */
/*                                                                               */
/* TASK    : show tft display update heating                                     */
/* UPDATE  : 22.09.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_update_battery(int battery) 
{
  // setting the battery
  M5.Lcd.fillRect(160,190,105,30,TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setFreeFont(FMB18);
  M5.Lcd.drawNumber(battery,200,190);
}

/*-------------------------------------------------------------------------------*/
/* Function void tft_display_m5stack(void)                                       */
/*                                                                               */
/* TASK    : show tft display m5stack (this m5stack)                             */
/* UPDATE  : 10.12.2020                                                          */
/*-------------------------------------------------------------------------------*/
void tft_display_m5stack(void) 
{
  // display room screen
  tft_display_room_screen("M5STACK-CORE-2",BLUE); 
  // display temperature sensor
  tft_display_sensor_temperature(m5stack_temp);
  // display humidity sensor
  tft_display_sensor_humidity(m5stack_humi); 
  // display pressure sensor
  tft_display_sensor_pressure(m5stack_press); 
  // display battery sensor
  tft_display_sensor_battery(m5stack_bat);
}

/*-------------------------------------------------------------------------------*/
/* Function void send_to_TTN(void)                                               */
/*                                                                               */
/* TASK    : send sensor data to TTN                                             */
/* UPDATE  : 25.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void send_to_TTN(void) 
{  
  // show picture
  iot_picture="/ttn.jpg";
  M5.Lcd.drawJpgFile(SD, iot_picture.c_str());
  
  // neopixels red
  rgb_neopixel(255,0,0);

  // activate communication
  Serial.println("LoraSet=?");  
  ATCommand("LoraSet", "?");
  delay(500);
  
  Serial.println(F(" "));
  Serial.print(F(application)); Serial.print(F(" Version ")); Serial.println(F(aktu_version));
  Serial.println(F(" "));

  // check if we can access battery functions
  if(!M5.Power.canControl())
  {
    Serial.println(F("[!] No communication with IP5306 chip"));
  }

  // actual battery level
  uint8_t bat = M5.Power.getBatteryLevel();
  Serial.print(F("[?] M5STACK BATTERY LEVEL --> "));
  Serial.print(bat);
  Serial.println(F(" %"));
  m5stack_bat=bat;
  int32_t battery_int = bat * 100; 
    
  #ifdef ENABLE_SENSOR_ENVII     
    sensor_env2();
    m5stack_temp=env2_tmp;
    m5stack_humi=env2_hum;
    m5stack_press=env2_pressure;
    Serial.print(F("[?] M5STACK Unit ENVII --> "));
    Serial.print("ENVII-P:"); Serial.print(env2_pressure); 
    Serial.print("  ENVII-T:"); Serial.print(env2_tmp);  
    Serial.print("  ENVII-H:"); Serial.println(env2_hum);                 
  #endif

  // now we create the payload and send it to the TTN
  int32_t temp_int      = env2_tmp * 100;
  int32_t pressure_int  = env2_pressure * 100;
  int32_t hum_int       = env2_hum * 100;

  byte payload[12];
  payload[0] = temp_int;
  payload[1] = temp_int >> 8;
  payload[2] = temp_int >> 16;

  payload[3] = hum_int;
  payload[4] = hum_int >> 8;
  payload[5] = hum_int >> 16;

  payload[6] = pressure_int;
  payload[7] = pressure_int >> 8;
  payload[8] = pressure_int >> 16;
    
  payload[9]  = battery_int;
  payload[10] = battery_int >> 8;
  payload[11] = battery_int >> 16;

  Serial.print(F("[x] actual TTN payload --> "));
  char str[32] = "";
  array_to_string(payload, 12, str);
  Serial.println(str);

  // now send all to TTN
  ATCommand("SendHex", str); 
  // neopixels now off
  rgb_neopixel_off();
  // display sensor data
  tft_display_m5stack();       
}

/*-------------------------------------------------------------------------------*/
/* Function void setup()                                                         */
/*                                                                               */
/* TASK    : setup all needed requirements                                       */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void setup() 
{
  // initialize the M5Stack object
  M5.begin();

  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();

  // open serial monitor
  Serial.begin(115200);
  
  // activate the NeoPixels
  pixels.begin();
  rgb_neopixel(0,0,0);

  // boot application
  delay(3000);
  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting..."));
  Serial.print(F(application)); Serial.print(F(" Version ")); Serial.println(F(aktu_version));
  Serial.println(F("connected via TTN Stuttgart"));
  Serial.println(F(" ")); 

  M5.Lcd.fillScreen(TFT_RED);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth("M5Stack CORE")) / 2, 100);
  M5.Lcd.print("M5Stack CORE");
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth("TTN SensorNode")) / 2, 120);
  M5.Lcd.print("TTN SensorNode");
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth("Version x.xx")) / 2, 140);
  M5.Lcd.print("Version ");
  M5.Lcd.print(aktu_version);
  delay(6000);

  // SD card
  M5.Lcd.fillScreen(TFT_NAVY);
  M5.Lcd.setFreeFont(FMB12);
  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setCursor((320 - M5.Lcd.textWidth("SD card initialize..")) / 2, 120);
  M5.Lcd.print("SD card initialize..");
  delay(2000);
  // check if we have a sd card inserted
  if (!SD.begin()) 
  {
    Serial.println("[!] SD Card failed, or not present");
    while (1);
  }
  Serial.println("[x] SD Card initialized.");

  // Lcd display 
  M5.Lcd.setBrightness(255); 

  // show iot picture
  iot_picture="/iot.jpg";
  M5.Lcd.drawJpgFile(SD, iot_picture.c_str());
  delay(2000);

  // activate the FIRE NeoPixels blue
  rgb_neopixel(0,0,255);

  // actual battery level
  uint8_t bat = M5.Power.getBatteryLevel();
  Serial.print(F("[?] M5STACK BATTERY LEVEL --> "));
  Serial.print(bat);
  Serial.println(F(" %"));
  m5stack_bat=bat;  

  #ifdef ENABLE_SENSOR_ENVII
    // show picture
    iot_picture="/unit_env2.jpg";
    M5.Lcd.drawJpgFile(SD, iot_picture.c_str());
    delay(2000);  
    
    Wire.begin();
    while (!bme.begin(0x76))
    {
      Serial.println(F("[!] Could not find a valid BMP280 sensor, check wiring!"));
    }
    Serial.println(F("[x] GROVE ENVII Sensor detected"));
    // read the env2 sensor */
    sensor_env2();
    m5stack_temp=env2_tmp;
    m5stack_humi=env2_hum;
    m5stack_press=env2_pressure;
    Serial.print(F("[?] M5STACK UNIT ENVII --> "));
    Serial.print("ENVII-P:"); Serial.print(env2_pressure); 
    Serial.print("  ENVII-T:"); Serial.print(env2_tmp);  
    Serial.print("  ENVII-H:"); Serial.println(env2_hum);  
  #endif

  #ifdef ENABLE_SENSOR_PIR_MOTION
    // show picture
    iot_picture="/unit_pir.jpg";
    M5.Lcd.drawJpgFile(SD, iot_picture.c_str());
    delay(2000);   
    
    pinMode(PIR_MOTION_SENSOR, INPUT);
    Serial.println(F("[x] GROVE PIR Motion Sensor detected"));
  #endif   
    
  // now connect to the M%Stack COM.LoRaWAN module
  // TX 0/3/17
  // RX 5/15/16
  // Before you connect the module to your M5Stack device, make sure you set the TXD/RXD dip switches correctly. 
  // If you use the grey or basic and have nothing else connected, the default UART pin configuration 16/17 is fine. 
  // For the Fire you should use 13/5, as all other settings can interfere with internals of the Fire.
  //   
  // Serial2.begin(115200, SERIAL_8N1, 15, 13);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  // show picture
  iot_picture="/comx_lorawan.jpg";
  M5.Lcd.drawJpgFile(SD, iot_picture.c_str());

  // we have to get and set some parameters 
  // first order does not work - waking module 
  ATCommand("LORAWAN", "?");
  delay(500);

  // setting LoRaWan Mode
  ATCommand("LORAWAN", "1");
  delay(500);

  // DevEui=XXXXXXXXXXXXXXXX(For OTAA Mode)
  // DevEui="XXXXXXXXXXXXXXXX";
   
  // AppEui=XXXXXXXXXXXXXXXX(For OTAA Mode)
  // always the same if equal devices
  // AppEui = "XXXXXXXXXXXXXXXX";
   
  // AppKey=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX(For OTAA Mode)
  // AppKey="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";  

  // your TTN access data
  ATCommand("DevEui", "0000AC33A5C40A24");
  delay(500);
  ATCommand("AppEui", "0000000000000000");
  delay(500);
  ATCommand("AppKey", "00112233445566778899AABBCCDDEEFF");
  delay(500);

  // we join the TTN network
  ATCommand("Join", "1");
  
  // show picture
  iot_picture="/ttn.jpg";
  M5.Lcd.drawJpgFile(SD, iot_picture.c_str());
  delay(2000);   
 
  // neopixels now off
  rgb_neopixel_off();

  // display sensor data
  tft_display_m5stack();   
}

/*-------------------------------------------------------------------------------*/
/* Function void loop()                                                          */
/*                                                                               */
/* TASK    : this runs forever                                                   */
/* UPDATE  : 24.01.2021                                                          */
/*-------------------------------------------------------------------------------*/
void loop()
{ 
  // check if some one has pressed a button
  if (M5.BtnA.wasPressed()) 
  {
    Serial.println("[x] Button A was pressed - Display ON/OFF");
    tft_backlight = !tft_backlight;   
    // Turning off the LCD backlight
    if (tft_backlight == false) { rgb_neopixel_off(); M5.Lcd.sleep();  M5.Lcd.setBrightness(0); }
    // Turning on the LCD backlight
    if (tft_backlight == true)  { M5.Lcd.wakeup(); M5.Lcd.setBrightness(255); }
    delay(200);
  }

  // check if some one has pressed a button
  if (M5.BtnB.wasPressed()) 
  {
    Serial.println("[x] Button B was pressed."); 
    delay(200);  
  }

  // check if some one has pressed a button
  if (M5.BtnC.wasPressed()) 
  {
    Serial.println("[x] Button C was pressed - Send to TTN");
    send_to_TTN();   
    delay(200);   
  }

  /*
   * If we have an enabled PIR Motion Sensor we will send immediately
   * a message to the LoRaWan Gateway if we have detected an intruder
   */ 
  #ifdef ENABLE_SENSOR_PIR_MOTION
    // read the motion sensor
    // only if display is dark
    if (tft_backlight == false)
    {
      int sensorValue = digitalRead(PIR_MOTION_SENSOR);
      // if the sensor value is HIGH we have an intruder ?
      if(sensorValue == HIGH)       
      { 
        if (MOTION == false)
        {
          // digitalWrite(LED, HIGH);
          MOTION=true;
          Serial.println("[x] PIR MOTION detected ..."); 
          // M5.Speaker.tone(661, 20);
          tft_backlight = true;  
          if (tft_backlight == true)  { tft_counter=0; M5.Lcd.wakeup(); M5.Lcd.setBrightness(255); }        
        }     
      }
      // if the sensor value is HIGH we have an intruder ?
      if(sensorValue == LOW)       
      { 
        // digitalWrite(LED, LOW);
        MOTION=false;   
      }
    }     
  #endif
  
  /* 
   * It is checked whether the time for the transmission interval has already expired
   * If the time difference between the last save and the current time is greater
   * as the interval, the following function is executed.
  */
  if (millis() - previousMillis > interval)
  {
    // correct timer
    previousMillis = millis();

    
    // shall we now publish (10 minutes)
    TTNCounter++;
    if (TTNCounter==20)
    {
      send_to_TTN();
      TTNCounter=0; 
    }
    
    // tft got to sleep after 1 minute
    tft_counter++;
    if (tft_counter==3) 
    {
      if (tft_backlight == true) { tft_backlight=false; }   
      // Turning off the LCD backlight
      if (tft_backlight == false) 
      {
        rgb_neopixel_off(); 
        M5.Lcd.sleep();  M5.Lcd.setBrightness(0);
        Serial.println(F("[x] sleeping mode ... "));
      }
      tft_counter=0;     
    }   
  }

  delay(100);  
  M5.update();
}
