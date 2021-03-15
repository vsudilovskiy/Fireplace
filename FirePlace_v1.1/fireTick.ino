//------------------------------------------------------------------------------------------------------------------------------------------------------
void fireTick() {
  static uint32_t prevTime, prevTime2;

  // задаём направление движения огня
  if (millis() - prevTime > 100) {
    prevTime = millis();
    FOR_i(0, ZONE_AMOUNT) { zoneRndValues[i] = random(0, 10); }
    
    FOR_i(0, ZONE_AMOUNT2) { zoneRndValues2[i] = random(0, 10); }

    FOR_i(0, ZONE_AMOUNT3) { zoneRndValues3[i] = random(0, 10); }

//    FOR_i(0, ZONE_AMOUNT4) { zoneRndValues4[i] = random(0, 10); }
    }
  

  // двигаем пламя
  if (millis() - prevTime2 > 20) {
    prevTime2 = millis();
    int thisPos = 0, lastPos = 0;
    
    FOR_i(0, ZONE_AMOUNT) {
      zoneValues[i] = (float)zoneValues[i] * (1 - SMOOTH_K) + (float)zoneRndValues[i] * 10 * SMOOTH_K;
      //zoneValues[i] = (zoneRndValues[i] * 10 - zoneValues[i]) * SMOOTH_K;
      leds[i] = 
          CHSV(
           vFireObject.HUE_START + map(zoneValues[i] , 20, 60, 0, vFireObject.HUE_GAP),                    // H
           constrain(map(zoneValues[i] , 20, 60, vFireObject.MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(zoneValues[i] , 20, 60, MIN_BRIGHT, vFireObject.MAX_BRIGHT), 0, 255)  // V
         );
      }
      
    FOR_i(0, ZONE_AMOUNT2) {
      zoneValues2[i] = (float)zoneValues2[i] * (1 - SMOOTH_K) + (float)zoneRndValues2[i] * 10 * SMOOTH_K;
      leds2[i] = 
          CHSV(
           vFireObject.HUE_START + map(zoneValues2[i] , 20, 60, 0, vFireObject.HUE_GAP),                    // H
           constrain(map(zoneValues2[i] , 20, 60, vFireObject.MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(zoneValues2[i] , 20, 60, MIN_BRIGHT, vFireObject.MAX_BRIGHT), 0, 255)  // V
         );
      }

    FOR_i(0, ZONE_AMOUNT3) {
      zoneValues3[i] = (float)zoneValues3[i] * (1 - SMOOTH_K) + (float)zoneRndValues3[i] * 10 * SMOOTH_K;
      leds3[i] = 
          CHSV(
           vFireObject.HUE_START + map(zoneValues3[i] , 20, 60, 0, vFireObject.HUE_GAP),                    // H
           constrain(map(zoneValues3[i] , 20, 60, vFireObject.MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(zoneValues3[i] , 20, 60, MIN_BRIGHT, vFireObject.MAX_BRIGHT), 0, 255)  // V
         );
      }      
      
//    FOR_i(0, ZONE_AMOUNT4) {
//      zoneValues4[i] = (float)zoneValues4[i] * (1 - SMOOTH_K) + (float)zoneRndValues4[i] * 10 * SMOOTH_K;
//      leds4[i] = 
//          CHSV(
//           vFireObject.HUE_START + map(zoneValues4[i] , 20, 60, 0, vFireObject.HUE_GAP),                    // H
//           constrain(map(zoneValues4[i] , 20, 60, vFireObject.MAX_SAT, MIN_SAT), 0, 255),       // S
//           constrain(map(zoneValues4[i] , 20, 60, MIN_BRIGHT, vFireObject.MAX_BRIGHT), 0, 255)  // V
//         );
//      }
      
    }
    FastLED.show(); 
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void changePower() {
    if (ONflag) { // Включение
        FastLED.setBrightness(255);
        digitalWrite(RELAY_PIN, LOW);
        delay(2000);
    } else { // Выключение
        FastLED.setBrightness(0);
        digitalWrite(RELAY_PIN, HIGH);
        delay(2000);
      }

    FastLED.clear();
    delay(2);
    FastLED.show();  
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void changeVolume() {
    String fName = "[changeVolume]->";
    debug_print(fName+"Begin");
    
    if(bMP3 == true) {
      int vVolume = map(vFireObject.MP3VOL,0,100,0,30);
      debug_print(fName+"Set voulme level="+String(vVolume));
      mp3.setVol(vVolume);
      mp3.qVol();   
    }
    debug_print(fName+"End");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void onMute() {
    String fName = "[onMute]->";
    debug_print(fName+"Begin");
    
    if(bMP3 == true) {
      if(vFireObject.ONMUTE == true) {
          debug_print(fName+"Stop MP3");
          mp3.stop();          
      }
      else {
          changeVolume();   //восстанавливаем значение уровня громности
          debug_print(fName+"Play MP3");
          mp3.playSL(1); // print decoded answers from mp3
      }        
    }
    debug_print(fName+"End");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
