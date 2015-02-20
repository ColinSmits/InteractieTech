
/*
===============
TallFont Header
===============
*/

#ifndef Menu_h
#define Menu_h

#include <LiquidCrystal.h>

void initializeMenu(LiquidCrystal *l);
void showIdleMenu();
bool resetMenu();
void activateMenu();
void selectMenu();

#endif