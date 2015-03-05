/*
  Button.cpp - Colling Smits & Bastiaan Weijers, feb 2015.


  Based on Morse.h, Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  
  Released into the public domain.
*/

#ifndef Button_h
#define Button_h

#include "Arduino.h"

class Button
{
  public:
    Button(int pin);
    bool read();
  private:
    int _pin;
    unsigned long _lastRead;
    bool state;
};

#endif