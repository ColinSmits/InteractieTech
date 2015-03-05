
/*
================
TallFont Library
================
*/

#include "Arduino.h"
#include "LiquidCrystal.h"
#include "Menu.h"

LiquidCrystal *output_lcd;

byte _idleDownArrow[8] = {
  0b00000,
  0b11111,
  0b10001,
  0b01010,
  0b01010,
  0b00100,
  0b00100,
  0b00000
};

byte _activeDownArrow[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b01110,
  0b01110,
  0b00100,
  0b00100,
  0b00000
};

byte _selectButton[8] = {
  B00000,
  B01110,
  B10001,
  B10101,
  B10101,
  B10001,
  B01110,
  B00000
};

bool state = false;
unsigned long lastmillis = 0;
int activeMenu = 0;
bool reset = false;
unsigned int _delay = 25000;
long resultBuilder[3] = {0, _delay, 0};
void initializeMenu(LiquidCrystal *l)
{
output_lcd=l;
output_lcd->createChar(0, _selectButton);
output_lcd->createChar(1, _idleDownArrow);
output_lcd->createChar(2, _activeDownArrow);

}

void showIdleMenu()
{
  output_lcd->clear();
  output_lcd->setCursor(15,0);
  output_lcd->write((uint8_t)0);
  output_lcd->setCursor(15,1);
  output_lcd->write((uint8_t)1);
  state = false;
}

void activateMenu(){
  activeMenu += 1;
  showIdleMenu();
  lastmillis = millis();
  state = true;
  switch (activeMenu % 2) {
    case 0:
      output_lcd->setCursor(0,0);
      output_lcd->print("Spray delay:");
      output_lcd->setCursor(0,1);
      output_lcd->print("Seconds: ");
      output_lcd->print(_delay/1000);
      //print one more empty space to write over possible previous double digits
      output_lcd->print(" ");
      break;
    case 1:
      output_lcd->setCursor(0,0);
      output_lcd->print("Reset count?");
      output_lcd->setCursor(0,1);
      reset = true;
      output_lcd->print("No  in ");
      break;
  }
}



void selectMenu(){
  if (state){
    lastmillis = millis();
    switch (activeMenu % 2) {
      case 0:
        _delay += 1000;
        if (_delay == 0){
	   _delay = 15000;
        }
        output_lcd->setCursor(0,1);
        output_lcd->print("Seconds: ");
        output_lcd->print(_delay/1000);
        //print one more empty space to write over possible previous double digits
        output_lcd->print(" ");
        break;
      case 1:
        output_lcd->setCursor(0,1);
        if (reset == true){
          output_lcd->print("Yes");
          reset = false;
        }else{
          output_lcd->print("No ");
          reset = true;
        }
        
        break;
    }
  }
}

long* resetMenu(){
  if (millis() - lastmillis > 5000){
    //active selected menu after 3 seconds
    output_lcd->clear();
    lastmillis = millis();
    showIdleMenu();
    resultBuilder[0] = 0;
    resultBuilder[1] = _delay; 
    if (reset){
       resultBuilder[2] = 1;
    }
    else {
       resultBuilder[2] = 0;
    }
    return resultBuilder;
  }
  if (activeMenu %2 == 1 && state){
    output_lcd->setCursor(10,1);
    output_lcd->print(5 - ((millis() - lastmillis) / 1000));
  }
  resultBuilder[0] = 1;
  return resultBuilder;
}









