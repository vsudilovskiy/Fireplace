// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#quick-start
// https://www.wemos.cc/en/latest/d1/d1_mini.html#pin

#include <SPI.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>


//---------------------------------------------------------------------
// настройки ленты
#define NUM_LEDS 10    // количество светодиодов
#define STRIP_PIN 2      // пин ленты

// настройки пламени
#define HUE_START 0     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define HUE_GAP  20     // коэффициент цвета огня (чем больше - тем дальше заброс по цвету)
#define SMOOTH_K 0.15   // коэффициент плавности огня
#define MIN_BRIGHT 140  // мин. яркость огня
#define MAX_BRIGHT 220  // макс. яркость огня
#define MIN_SAT 220     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

// для разработчиков
#define ZONE_AMOUNT NUM_LEDS   // количество зон
byte zoneValues[ZONE_AMOUNT];
byte zoneRndValues[ZONE_AMOUNT];

//#define ORDER_GRB       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
#define COLOR_DEBTH 3   // цветовая глубина: 1, 2, 3 (в байтах)
// на меньшем цветовом разрешении скетч будет занимать в разы меньше места,
// но уменьшится и количество оттенков и уровней яркости!

// ВНИМАНИЕ! define настройки (ORDER_GRB и COLOR_DEBTH) делаются до подключения библиотеки!
#include <microLED.h>
#include <FastLEDsupport.h> // вкл поддержку

//LEDdata leds[NUM_LEDS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
//microLED strip(leds, NUM_LEDS, LED_PIN);  // объект лента
microLED< NUM_LEDS, STRIP_PIN, -1, LED_WS2811, ORDER_GBR> strip; //ORDER_GBR
// ленивая жопа
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
//---------------------------------------------------------------------
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 200);
IPAddress mqttserver(192, 168, 0, 180);
//---------------------------------------------------------------------
EthernetClient ethClient;
PubSubClient mqttclient(ethClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
//---------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  strip.setBrightness(255);
  mqttclient.setServer(mqttserver, 1883);
  mqttclient.setCallback(mqttcallback);
  
  Ethernet.begin(mac);
  // Allow the hardware to sort itself out
  delay(1500);
  Serial.println(Ethernet.localIP());
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  FastLED.clear(); 
  
  if (!mqttclient.connected()) {
    mqttreconnect();
  }
  mqttclient.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttclient.publish("outTopic", msg);
  }
    fireTick();
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
