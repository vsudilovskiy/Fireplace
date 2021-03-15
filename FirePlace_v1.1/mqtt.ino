//------------------------------------------------------------------------------------------------------------------------------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String fName = "[mqttCallback]->";
    debug_print(fName+"Begin");
    String Payload = "";
    bool bSaveFlash = false;    //записываем параметры на Flash только если получили новые значения из MQTT, иначе будем постоянно дергать память, так как сами шлем infoCallback
    byte vTempVal;
    bool vTempBool;
    
   // debug_print(fName+"Message arrived ["+String(topic)+"]");
//    #ifdef DEBUG
//    Serial.print("[callback] Message arrived [");
//    Serial.print(topic);
//    Serial.print("] ");
//    #endif
  
    for (int i = 0; i < length; i++) Payload += (char)payload[i];
    debug_print(fName+"Message arrived ["+String(topic)+"] "+String(Payload));
//    #ifdef DEBUG
//    Serial.println(Payload);
//    #endif
  
  
    if (String(topic) == "homeassistant/fireplace/switch") {
        ONflag = (Payload == "ON") ? true : false;
        debug_print(fName+"ONflag="+Payload);
//        Serial.println("[callback] ONflag="+Payload);

        if(ONflagLast != ONflag) { //если пришло новое значение
            debug_print(fName+"Change ONflag");
//            Serial.println("[callback] Change ONflag");
            vFireObject.ONMUTE = !ONflag; //если включаем устроство, то выключаем mute и наоборот
            changePower();    //включаем или выключаем ленту
 //           changeVolume();   //восстанавливаем значение уровня громности
            onMute();   //запускаем или осанавливаем проигрываение
            ONflagLast = ONflag;    //запоминаем последний статус
        }
    }
  
    //остальные команды обрабатывем только если устройство включено
    if(ONflag) {
        if (String(topic) == "homeassistant/fireplace/huemode") {
            vTempVal = Payload.toInt();
            bSaveFlash = (vFireObject.HUE_START != vTempVal) ? true : false;
            vFireObject.HUE_START = vTempVal;
        }
        if (String(topic) == "homeassistant/fireplace/huegap") {
            vTempVal = Payload.toInt();
            bSaveFlash = (vFireObject.HUE_GAP != vTempVal) ? true : false;
            vFireObject.HUE_GAP = vTempVal;
        }
        if (String(topic) == "homeassistant/fireplace/maxbright") {
            vTempVal = Payload.toInt();
            bSaveFlash = (vFireObject.MAX_BRIGHT != vTempVal) ? true : false;
            vFireObject.MAX_BRIGHT = constrain(vTempVal,140, 240);
        }
        if (String(topic) == "homeassistant/fireplace/maxsat") {
            vTempVal = Payload.toInt();
            bSaveFlash = (vFireObject.MAX_SAT != vTempVal) ? true : false;
            vFireObject.MAX_SAT = constrain(vTempVal,220, 255);
        }
        if (String(topic) == "homeassistant/fireplace/volume")  {
            vTempVal = Payload.toInt();
            
            if(vFireObject.MP3VOL != vTempVal) {
                bSaveFlash = true;
                vFireObject.MP3VOL = vTempVal;
                changeVolume();
            } 
        }
        if (String(topic) == "homeassistant/fireplace/mute") {   //хранить значение не принципиально, так как при запуске не используется
            vTempBool = (Payload == "ON") ? true : false;
            if(vFireObject.ONMUTE != vTempBool) {
                vFireObject.ONMUTE = vTempBool;
                onMute();              
            }
        }   

        if(bSaveFlash) saveEEPROM();
    }
    debug_print(fName+"End");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void initCallback() { //при первом запуске отправляем параетры в 
    return;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void infoCallback() { //если по каким то причинам в MQTT параметры потерлись, их нужно отправлять на периодической основе
    String fName = "[infoCallback]->";
    debug_print(fName+"Begin");
    if(bFirstLoad) {
        bFirstLoad = false;
        mqttclient.publish(String("homeassistant/fireplace/switch").c_str(), String("OFF").c_str(), true);
    }
    else {
        mqttclient.publish(String("homeassistant/fireplace/switch").c_str(), String((ONflag == true) ? "ON" : "OFF").c_str(), true);
    }
    mqttclient.publish(String("homeassistant/fireplace/huemode").c_str(), String(vFireObject.HUE_START).c_str(), true);
    mqttclient.publish(String("homeassistant/fireplace/huegap").c_str(), String(vFireObject.HUE_GAP).c_str(), true);
    mqttclient.publish(String("homeassistant/fireplace/maxbright").c_str(), String(vFireObject.MAX_BRIGHT).c_str(), true);
    mqttclient.publish(String("homeassistant/fireplace/maxsat").c_str(), String(vFireObject.MAX_SAT).c_str(), true);
    mqttclient.publish(String("homeassistant/fireplace/volume").c_str(), String(vFireObject.MP3VOL).c_str(), true);
    mqttclient.publish(String("homeassistant/fireplace/mute").c_str(), String((vFireObject.ONMUTE == true) ? "ON" : "OFF").c_str(), true);

#ifdef DHTTYPE

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    mqttclient.publish(String("homeassistant/livingroom/tempre").c_str(), String(event.temperature).c_str(), true);
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    mqttclient.publish(String("homeassistant/livingroom/hmdty").c_str(), String(event.relative_humidity).c_str(), true);   
  }
#endif

//  if(bMP3 == true) 
//      mqttclient.publish(String("homeassistant/fireplace/debug").c_str(), String("mp3.available").c_str(), true);
//  else
//      mqttclient.publish(String("homeassistant/fireplace/debug").c_str(), String("mp3.NOT! available").c_str(), true);

  debug_print(fName+"End");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 struct sFireObject{
  byte HUE_START = 0; 
  byte HUE_GAP = 20;
  byte MAX_BRIGHT = 220;
  byte MAX_SAT = 255;
  byte MP3VOL = 30;
  bool ONMUTE = false;
} vFireObject;

 */


void mqttreconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttclient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttclient.subscribe("homeassistant/fireplace/switch"); //ON OFF
      mqttclient.subscribe("homeassistant/fireplace/huemode"); // начальный цвет огня [0 красный, 80 зелёный, 140 молния, 190 розовый]
      mqttclient.subscribe("homeassistant/fireplace/huegap"); // коэффициент цвета огня (чем больше - тем дальше заброс по цвету) [20,30,40,50]
      mqttclient.subscribe("homeassistant/fireplace/maxbright"); // макс. яркость огня [>= 140 <= 240]
      mqttclient.subscribe("homeassistant/fireplace/maxsat"); // макс. насыщенность
      mqttclient.subscribe("homeassistant/fireplace/volume"); //0 - 100 / step 10
      mqttclient.subscribe("homeassistant/fireplace/mute"); //ON OFF
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
