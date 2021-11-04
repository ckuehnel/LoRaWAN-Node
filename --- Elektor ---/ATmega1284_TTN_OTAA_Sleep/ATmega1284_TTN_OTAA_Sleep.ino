/*
 * File: ATmega1284_TTN_OTAA_Sleep.ino
 * 
 * based on https://project-ml.de/ttn-lora-board-v3-atmega1284/
 * adapted 2021-07-08 by Claus Kühnel info@ckuehnel.ch
 * 
 * used libraries
 * - https://github.com/mcci-catena/arduino-lmic
 * - https://github.com/cpldcpu/light_ws2812
 * - https://github.com/jeremycole/Temperature_LM75_Derived
 * - https://github.com/LowPowerLab/LowPower
 * - https://github.com/SodaqMoja/Sodaq_SHT2x
 */

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <LowPower.h>

bool next = false;

//Externer Sensor
#include <Sodaq_SHT2x.h>

//Neopix LED Light2812
#include <WS2812.h>
WS2812 LED(1); // 1 LED
cRGB value;

// LM75 Temperatur Sensor I2C
#include <Temperature_LM75_Derived.h>
Generic_LM75 LM75;

//Stromversorgung NEO, LM, I2C
const int VEXT = 19;  //Digital Pin 19/PC3
const int CHRG = 18;  // PC2/18 Ladeanzeige Pin
int LadA = 1;         // save read value

// DEBUB Schalter, uncomment for Serial enabled
#define DEBUG

// Accu Selection, uncommented Lipo else LiFePo
//#define LiFePo

uint16_t volt, exhum;
int16_t  itmp, extmp;


//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
//# define COMPILE_REGRESSION_TEST //comment this out if you want to compile this sketch with 'FILLMEIN' values for APPEUI, DEVEUI and APPKEY
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x84, 0x0B, 0x22, 0x00, 0x0B, 0xA3, 0x04, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xCB, 0x6E, 0x85, 0x47, 0x4C, 0x87, 0xF9, 0x83, 0x31, 0xFA, 0xAB, 0x13, 0x09, 0x19, 0x3E, 0x7B };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[8];
static osjob_t sendjob;

// Sleep Time in Seconds
const unsigned TX_INTERVAL = 300;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 14,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 13,
    .dio = {10, 11, 12},
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16) Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("NetId: ");
              Serial.println(netid, DEC);
              Serial.print("DevAddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) 
              {
                if (i != 0) Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) 
              {
                if (i != 0) Serial.print("-");
                printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	          // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            next = true;
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

//***************************Daten generierung und Senden***********************************
void do_send(osjob_t* j)
{
  #ifdef DEBUG
  Serial.println(F("Start do_send"));
  #endif

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) Serial.println(F("OP_TXRXPEND, not sending"));
  else 
  {
    // Get sensor values
    getValues();
    
    // Prepare upstream data transmission at the next possible time.
    mydata[0] = highByte(volt); mydata[1] = lowByte(volt);
    mydata[2] = highByte(itmp); mydata[3] = lowByte(itmp);
    mydata[4] = highByte(extmp); mydata[5] = lowByte(extmp);
    mydata[6] = highByte(exhum); mydata[7] = lowByte(exhum);
    
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() 
{
  Wire.begin();
  pinMode(CHRG, INPUT); 
  pinMode(VEXT, OUTPUT);  
  
  // Debug Messages
  #ifdef DEBUG
    Serial.begin(9600);
    Serial.println(F(" "));
    Serial.println(F("Start HSN-Node XX"));
    Serial.print(F("CPU ATmega1284P "));
    Serial.print(F("Node "));

    // Akku
    #ifdef LiFePo
      Serial.println(F("w/ LiFePo4 Accu"));
    #else
      Serial.println(F("w/ Li-Po Accu"));
    #endif
  #endif

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() 
{
  extern volatile unsigned long timer0_overflow_count;
  if (next == false) os_runloop_once();
  else 
  {
    Serial.println(F("Node goes to sleep."));
    Serial.flush(); // give the serial print chance to complete
    int sleepCycles = TX_INTERVAL / 8; // how often we go to sleep for 8 seconds
    for (int i = 0; i < sleepCycles; i++) 
    {
      // Enter power down state for 8 s with ADC and BOD module disabled
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

      // LMIC uses micros() to keep track of the duty cycle, so
      // hack timer0_overflow for a rude adjustment:
      cli();
      timer0_overflow_count += 8 * 64 * clockCyclesPerMicrosecond();
      sei();
    }
    next = false;
    // Start job
    do_send(&sendjob);
  }
}
