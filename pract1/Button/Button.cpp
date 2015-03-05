/*
  Button.cpp - Colling Smits & Bastiaan Weijers, feb 2015.

  Based on Morse.cpp, Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  
  Released into the public domain.
*/

#include "Arduino.h"
#include "Button.h"

Button::Button(int pin)
{
  pinMode(pin, INPUT);
  _pin = pin;
  _lastRead = 0;
  state = false;
}

bool Button::read(){

  if (digitalRead(_pin) == HIGH){
    if (state){
      return false;
    }
    else{
      state = true;
      _lastRead = millis();
      return true;
    }
  }else{
    if (state && (millis() - _lastRead >= 100)){
      //100ms has passed since active, set to false
      state = false;
      return false;
    }else{
      return false;
    }
  }
}