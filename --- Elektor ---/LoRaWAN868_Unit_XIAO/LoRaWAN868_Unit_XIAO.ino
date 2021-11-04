/*
 * File: LoRaWAN868 Unit XIAO.ino
 * 
 * Seeedstudioo XIAO w/ ENV.II Unit sends data by M5Stack LoRaWAN868 Unit
 * based on https://raw.githubusercontent.com/m5stack/M5Stack/master/examples/Unit/LoRaWAN868/LoRaWAN868.ino
 * 
 * 2021-06-30 Claus Kühnel info@ckuehnel.ch
 */
#define DEBUG 1

float sht31Temperature, sht31Humidity;
float bmp280Temperature, bmp280Pressure;

const long CYCLE = 15 * 60000; // Transmission cycle 15 minutes
unsigned long previousMillis = 0;

String waitRevice()
{
    String recvStr;
    do
    {
        recvStr = Serial1.readStringUntil('\n');
    } while (recvStr.length() == 0);
    Serial.println(recvStr);
    return recvStr;
}

void sendATCMD(String cmdStr)
{
    Serial1.print(cmdStr);
    delay(10);
}

int sendATCMDAndRevice(String cmdStr)
{
    Serial1.print(cmdStr);
    delay(10);
    waitRevice();
    String recvStr = waitRevice();
    if (recvStr.indexOf("OK") != -1) return 0;
    else  return -1;
}

void setup()
{
  SerialUSB.begin(115200);
  uint32_t baud = SerialUSB.baud();
  Serial1.begin(baud);
  while (!Serial);
  while (!SerialUSB);
  SerialUSB.println("Seeedstudion XIAO sends ENV.II Unit data by M5Stack LoRaWAN868 Unit");
  Serial1.flush();
  SerialUSB.println("ENV.II Unit Initialization...");
  initSensor();
  delay(100);
  sendATCMDAndRevice("AT+CGMI?\r");
  sendATCMDAndRevice("AT+CGMM?\r");
  delay(100);
  Serial1.flush();
  sendATCMDAndRevice("AT+ILOGLVL=0\r");
  sendATCMDAndRevice("AT+CSAVE\r");
  sendATCMDAndRevice("AT+IREBOOT=0\r");
  SerialUSB.println("LoraWan Rebooting");
  delay(2000);
  SerialUSB.println("LoraWan Configuration...");
  sendATCMDAndRevice("AT+CJOINMODE=0\r");
  sendATCMDAndRevice("AT+CDEVEUI=006732A6EA7B908D\r");
  sendATCMDAndRevice("AT+CAPPEUI=70B3D57ED004247E\r");
  sendATCMDAndRevice("AT+CAPPKEY=DF9866BF31F695A9350B282906BB3B58\r");
  sendATCMDAndRevice("AT+CULDLMODE=2\r");
  sendATCMDAndRevice("AT+CCLASS=2\r");
  sendATCMDAndRevice("AT+CWORKMODE=2\r");
  sendATCMDAndRevice("AT+CNBTRIALS=0,5\r");
  sendATCMDAndRevice("AT+CNBTRIALS=1,5\r");


  // TX Freq
  // 868.1 - SF7BW125 to SF12BW125
  // 868.3 - SF7BW125 to SF12BW125 and SF7BW250
  // 868.5 - SF7BW125 to SF12BW125
  // 867.1 - SF7BW125 to SF12BW125
  // 867.3 - SF7BW125 to SF12BW125
  // 867.5 - SF7BW125 to SF12BW125
  // 867.7 - SF7BW125 to SF12BW125
  // 867.9 - SF7BW125 to SF12BW125
  // 868.8 - FSK

  sendATCMDAndRevice("AT+CFREQBANDMASK=0001\r");

  //869.525 - SF9BW125 (RX2)              | 869525000
  sendATCMDAndRevice("AT+CRXP=0,0,869525000\r");

  sendATCMDAndRevice("AT+CSAVE\r");
  
  sendATCMDAndRevice("AT+CJOIN=1,1,10,8\r");
}

enum systemstate
{
    kIdel = 0,
    kJoined,
    kSending,
    kWaitSend,
    kEnd,
};

int system_fsm = kIdel;

int loraWanSendNUM = -1;
int loraWanSendCNT = -1;
int loraWanupLinkCNT = 0;
int loraWanupLinkReviceCNT = 0;

void loop()
{
  String recvStr = waitRevice();
  if (recvStr.indexOf("+CJOIN:") != -1)
  {
    if (recvStr.indexOf("OK") != -1)
    {
      SerialUSB.println("[ INFO ] JOIN IN SUCCESSFUL");
      system_fsm = kJoined;
    }
    else
    {
      SerialUSB.println("[ INFO ] JOIN IN FAIL");
      system_fsm = kIdel;
    }
  }
  else if (recvStr.indexOf("OK+RECV") != -1)
  {
    if (system_fsm == kJoined) system_fsm = kSending;
    else if (system_fsm == kWaitSend)
    {
      //system_fsm = kEnd;
      char strbuff[128];
      loraWanupLinkReviceCNT ++;

      delay(500);
      system_fsm = kSending;
    }
  }
  else if(recvStr.indexOf("OK+SEND") != -1)
  {
    String snednum = recvStr.substring(8);
        
    SerialUSB.printf("[ INFO ] SEND NUM %s \r\n",snednum.c_str());
    loraWanSendNUM = snednum.toInt();
  }
  else if(recvStr.indexOf("OK+SENT") != -1)
  {
    String snedcnt = recvStr.substring(8);
    SerialUSB.printf("[ INFO ] SEND CNT %s \r\n",snedcnt.c_str());
    loraWanSendCNT = snedcnt.toInt();
  }
  else if(recvStr.indexOf("ERR+SENT") != -1)
  {
    char strbuff[128];
    String ErrorCodeStr = recvStr.substring(9);
    SerialUSB.printf(strbuff,"ERROR Code:%d (%d/%d)",ErrorCodeStr.toInt(),loraWanupLinkReviceCNT,loraWanupLinkCNT);
    delay(500);
    system_fsm = kSending;
  }
  else if(recvStr.indexOf("+CLINKCHECK:") != -1)
  {
    String checkStr = recvStr.substring(String("+CLINKCHECK:").length());
    SerialUSB.println(checkStr);
    SerialUSB.println(loraWanupLinkReviceCNT);;
    SerialUSB.println(loraWanupLinkCNT);
  }
  if (system_fsm == kSending)
  {
    SerialUSB.println("\nLoraWan Sending...");
    
    getValues(); // get measuring results of ENV.II sensor

    // prepare the measuring results for use in payload
    // you can enhance payload w/ BMP280 data, if you want
    int16_t temp = (int16_t) (sht31Temperature * 100. + .5);
    int16_t humi = (int16_t) (sht31Humidity * 10. + .5);
    
    char buffer [2];
    char command[] = "AT+DTRX=1,2,4,";
    if (temp < 0x1000) strcat(command, "0");
    strcat(command, itoa(temp, buffer, 16));
    if (humi < 0x1000) strcat(command, "0");
    strcat(command, itoa(humi, buffer, 16));
    strcat(command, "\r");
    SerialUSB.print("Send Command: ");
    SerialUSB.println(command);
 
    sendATCMD(command);
    //sendATCMD("AT+DTRX=1,2,4,65666768\r");
    sendATCMD("AT+CLINKCHECK=1\r");
    loraWanupLinkCNT ++;
    system_fsm = kWaitSend;
  }
  delay(36000);
}
