// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#quick-start
// https://www.wemos.cc/en/latest/d1/d1_mini.html#pin

//https://arduinomaster.ru/datchiki-arduino/datchiki-temperatury-i-vlazhnosti-dht11-dht22/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define DEBUG
#define INIT_ADDR 10                // номер резервной ячейки
#define ARRAY_ADDR 200              // номер резервной ячейки
#define INIT_KEY 55                 // ключ первого запуска. 0-254, на выбор (если вам нужно перезаписать массив во Flash, измените значение ключа и залейте скетч)
//---------------------------------------------------------------------
#include <FastLED.h>                // вкл поддержку
// ленивая жопа
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)

// настройки пламени
struct sFireObject{
  byte HUE_START = 0;               // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
  byte HUE_GAP = 20;                // коэффициент цвета огня (чем больше - тем дальше заброс по цвету)
  byte MAX_BRIGHT = 220;            // макс. яркость огня
  byte MAX_SAT = 255;               // мин. насыщенность
  byte MP3VOL = 30;
  bool ONMUTE = false;
} vFireObject;
#define SMOOTH_K 0.15               // коэффициент плавности огня
#define MIN_BRIGHT 140              // мин. яркость огня
#define MIN_SAT 220                 // мин. насыщенность
#define COLOR_DEBTH 3               // цветовая глубина: 1, 2, 3 (в байтах)
#define COLOR_ORDER BRG 
//---------------------------------------------------------------------
// настройки ленты1
#define STRIP_PIN 2                 // пин ленты D4
#define NUM_LEDS 13                 // количество светодиодов (1)
CRGB leds[NUM_LEDS];
#define ZONE_AMOUNT NUM_LEDS        // количество зон
byte zoneValues[ZONE_AMOUNT];
byte zoneRndValues[ZONE_AMOUNT];

// настройки ленты2
#define STRIP_PIN2 0                 // пин ленты D3
#define NUM_LEDS2 6                 // количество светодиодов (1)
CRGB leds2[NUM_LEDS2];
#define ZONE_AMOUNT2 NUM_LEDS2        // количество зон
byte zoneValues2[ZONE_AMOUNT2];
byte zoneRndValues2[ZONE_AMOUNT2];

// настройки ленты3
#define STRIP_PIN3 4                 // пин ленты D2
#define NUM_LEDS3 10                 // количество светодиодов (1)
CRGB leds3[NUM_LEDS3];
#define ZONE_AMOUNT3 NUM_LEDS3        // количество зон
byte zoneValues3[ZONE_AMOUNT3];
byte zoneRndValues3[ZONE_AMOUNT3];

//// настройки ленты4
//#define STRIP_PIN4 5                 // пин ленты D1
//#define NUM_LEDS4 5                 // количество светодиодов (1)
//CRGB leds4[NUM_LEDS4];
//#define ZONE_AMOUNT4 NUM_LEDS4        // количество зон
//byte zoneValues4[ZONE_AMOUNT4];
//byte zoneRndValues4[ZONE_AMOUNT4];

//---------------------------------------------------------------------
// WIFI + MQTT
const char* ssid = "WIFI";
const char* password = "******";
const char* mqtt_server = "192.168.0.*";
//---------------------------------------------------------------------
WiFiClient espClient;
PubSubClient mqttclient(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
//---------------------------------------------------------------------
// 
bool ONflag = false;                                  //статус устройства.
bool ONflagLast = false;                              //статус устройства (контроль нового значения ерез MQTT).
bool bFirstRun = false;                               //первый запуск устройства после прошивки
bool bFirstLoad = true;                              //первая загрузка после включения
bool bMP3 = false;

//
//int address = 0;                                    // Переменная для хранения адреса
//byte value1;                                        // Переменная для хранения значения
//---------------------------------------------------------------------
//File mp3
// Pin 15,13 must be disconnected during program upload.
#include "SerialMP3Player.h"
#define TX 15 //D8
#define RX 13 //D7
SerialMP3Player mp3(RX,TX);
//bool bMP3ready = false;
//---------------------------------------------------------------------
//DHT22
#define DHTTYPE DHT11

#ifdef DHTTYPE
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 12                                     //D6
#define DHTTYPE DHT11                                 // DHT 22  (AM2302), AM2321
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
#endif
//---------------------------------------------------------------------

#define RELAY_PIN 14
//---------------------------------------------------------------------
//Timer
#include <Timer.h>
Timer *mqttTimer = new Timer(6000);     //60000             //отправка параметров в mqtt по таймеру
//------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  byte vtmp;
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(RELAY_PIN, OUTPUT); 
  
  debug_print("setup->Begin");
  Serial.println(EEPROM.length());

  dhtsetup();
  
  if(EEPROM.read(INIT_ADDR) != INIT_KEY) {
      debug_print("[!]-First run");
      EEPROM.write(INIT_ADDR,INIT_KEY);    
      saveEEPROM();
      bFirstRun = true;
  }
  else { 
      Serial.println("[!]-Read vFireObject");
      EEPROM.get(ARRAY_ADDR,vFireObject);
    }


  mp3.begin(9600);        // start mp3-communication
  delay(500);             // wait for init
  mp3.sendCommand(CMD_SEL_DEV, 0, 2);   //select sd-card
  delay(500);             // wait for init

  if (mp3.available()){
    bMP3 = true;
    changeVolume();
  }
  
  FastLED.addLeds<WS2811, STRIP_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.addLeds<WS2811, STRIP_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.addLeds<WS2811, STRIP_PIN3, COLOR_ORDER>(leds3, NUM_LEDS3)/*.setCorrection( TypicalLEDStrip )*/;
//  FastLED.addLeds<WS2811, STRIP_PIN4, COLOR_ORDER>(leds4, NUM_LEDS4)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.clear();
  FastLED.setBrightness(0);
  delay(2);
  FastLED.show(); 

  setup_wifi();
  setupOTA();
  mqttclient.setServer(mqtt_server, 1883);
  mqttclient.setCallback(mqttCallback);
  
  mqttTimer->setOnTimer(&infoCallback);
  mqttTimer->Start();
  changePower();    //выключаем ленту
  Serial.println("setup->End");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {  
    mqttTimer->Update();
    ArduinoOTA.handle();
  
    if (!mqttclient.connected()) {
        mqttreconnect();
    }
    
    mqttclient.loop();

    if(ONflag)
        fireTick();
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
