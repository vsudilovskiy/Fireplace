void fireTick() {
  static uint32_t prevTime, prevTime2;

  // задаём направление движения огня
  if (millis() - prevTime > 100) {
    prevTime = millis();
    FOR_i(0, ZONE_AMOUNT) {
      zoneRndValues[i] = random(0, 10);
    }
  }

  // двигаем пламя
  if (millis() - prevTime2 > 20) {
    prevTime2 = millis();
    int thisPos = 0, lastPos = 0;
    FOR_i(0, ZONE_AMOUNT) {
      zoneValues[i] = (float)zoneValues[i] * (1 - SMOOTH_K) + (float)zoneRndValues[i] * 10 * SMOOTH_K;
      //zoneValues[i] = (zoneRndValues[i] * 10 - zoneValues[i]) * SMOOTH_K;
      strip.leds[i] = 
          mHSV(
           HUE_START + map(zoneValues[i] , 20, 60, 0, HUE_GAP),                    // H
           constrain(map(zoneValues[i] , 20, 60, MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(zoneValues[i] , 20, 60, MIN_BRIGHT, MAX_BRIGHT), 0, 255)  // V
         );
}
    }
    
    /*strip.leds[0] = mRed; //GREEN
    strip.leds[1] = mGreen; //RED
    strip.leds[2] = mBlue; 
    strip.leds[3] = mYellow; 
    strip.leds[4] = mBlack; 
    */
    strip.show(); 
}
