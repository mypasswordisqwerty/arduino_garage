#include <EasyButton.h>
#include <LedGroup.h>
#include <EEPROM.h>

int LOOP_DELAY = 100;
int DELAY_TIME = 5000;
int PRESS_TIME = 3000;
int BLINK_TIME = 1000;

int MODE_OFF = 0;
int MODE_BLINK = 1;
int MODE_ON = 2;

int pins[] = {4,5,6,-1};
int lightConf = 7;
Led foreLamp(7);
Led blinkLight(9);
LedGroup lamps(pins);
EasyButton light(0);
EasyButton fore(1);
EasyButton door(2);
EasyButton gate(3);
int lightMode = MODE_OFF;
int foreMode = MODE_OFF;
int lightConfig = 0;
int configDelay = 0;
int blinkDelay = 0;

void foreSwitch(){
  if (foreMode < MODE_ON){
    foreMode = MODE_ON;
  }else{
    foreMode = gate.isReleased() ? MODE_BLINK : MODE_OFF;
  }
  foreLamp.set(foreMode!=MODE_OFF);
}

void lightSwitch(){
  if (lightConfig){
    lightConf = lamps.getState()+1;
    if (lightConf>7 || lightConf==0) lightConf=1;
    lamps.setState(lightConf);
    configDelay = DELAY_TIME;
    return;
  }
  if(lightMode < MODE_ON){
    lightMode = MODE_ON;
  }else{
    lightMode = gate.isReleased() || door.isReleased() ? MODE_BLINK : MODE_OFF;
  }
  lamps.setState(lightMode==MODE_OFF ? 0 : lightConf);
}

void confChange() {
  lamps.setState(0);
  delay(500);
  lamps.setState(7);
  delay(500);
  lamps.setState(0);
  delay(500);
  lamps.setState(lightConf);
  if (lightConfig==0){
    lightConfig=1;
    configDelay = DELAY_TIME;
  }else{
    EEPROM.put(0, lightConf);
    lightConfig=configDelay=0;
  }
}

void setup() {
  // put your setup code here, to run once:
  EEPROM.get(0, lightConf);
  blinkLight.on();
  fore.onPressed(foreSwitch);
  light.onPressed(lightSwitch);
  light.onPressedFor(PRESS_TIME, confChange);
  fore.begin();
  light.begin();
  door.begin();
  gate.begin();
}

void checkDoors(){
  if (gate.isReleased() || door.isReleased()){
    if (lightMode==MODE_OFF){
      lamps.setState(lightConf);
      lightMode = MODE_BLINK;
    }
    if (foreMode==MODE_OFF && gate.isReleased()){
      foreLamp.on();
      foreMode = MODE_BLINK;
    }
    if (foreMode==MODE_BLINK && !gate.isReleased()){
      foreLamp.off();
      foreMode = MODE_OFF;
    }
    return;
  }
  if (lightMode==MODE_BLINK){
    lamps.setState(0);
    lightMode = MODE_OFF;
  }
  if (foreMode==MODE_BLINK){
    foreLamp.off();
    foreMode = MODE_OFF;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  fore.read();
  light.read();
  door.read();
  gate.read();
  checkDoors();
  if (lightMode==MODE_BLINK || foreMode==MODE_BLINK){
     blinkDelay-=LOOP_DELAY;
     if (blinkDelay<=0){
        blinkLight.change();
        blinkDelay = BLINK_TIME;
     }
  }else{
    blinkLight.on();
  }
  if (configDelay>0 && lightConfig>0){
    configDelay-=LOOP_DELAY;
    if (configDelay<=0){
      confChange();
    }
  }
  delay(LOOP_DELAY);
}
