#include <OneWire.h>
#include <DallasTemperature.h>
#include <Menu.h>
#include <Button.h>
#include <EEPROM.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
Button menuButton(15); //A3
Button choiceButton(16); //A4
Button sprayButton(17); // A5
int amountOfSpraysLeft = EEPROM.read(1);

#define echoPin 4// Echo Pin
#define trigPin 3// Trigger Pin
#define ledPin 13 // Onboard LED
#define motionPin 5 // Motion Pin
#define LDRPin 0 // Light Sensor
#define magnetPin 6 // Magnetic sensor
#define greenLED 19 //
#define redLED 18 //

int menuActivatedCount = 0;
int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, d; // Duration used to calculate distance
unsigned long previousMillisMotion = 0;
unsigned long previousMillisDist = 0;
unsigned long nr1Timer = 0;
unsigned long nr2Timer = 0;
unsigned long menuTimer = 0;
unsigned long prevTempTime = 0;
unsigned long tempDelay = 2000;
unsigned long triggertime = 10000; // Set time (ms) you need to have no motion to change state
unsigned long distanceDelay = 250;
unsigned long usageDelay = 5000;
unsigned long lastUsed = -usageDelay;
unsigned long ledDelay = 15000;
unsigned long extraDelay = 2500;
int minDistance = 25;
unsigned long sprayTime = 15000;
unsigned long sprayDelay = 25000;
unsigned long decisionTime = 2000;
unsigned long sprayButTimer = 0;
int ledState = HIGH;         // the current state of the output pin
int magnetState;             // the current reading from the input pin
int lastMagnetState = LOW;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;

long redTimer = 0;
long greenTimer = 0;
bool redOn = false;
bool greenOn = false;

//Distance helper variables
bool sitting = false;
bool standing = false;
bool attemptToSit = false;
bool attemptToStand = false;
bool attemptToUse = false;
bool attemptToFlee = false;
bool nr1Done = false;
bool nr2Done = false;
bool nr1Light = false;
bool nr2Light = false;
bool sprayButtonTouched = false;

unsigned long sitAttemptTime = 0;
unsigned long standAttemptTime = 0;
unsigned long useAttemptTime = 0;
unsigned long standTime = 0;
unsigned long sitTime = 0;
unsigned long useTime = 0;
unsigned long maxSitTime = 0;
unsigned long maxStandTime = 0;
unsigned long maxUseTime = 0;
unsigned long previousDistState = 0;
unsigned long fleeAttemptTime = 0;
unsigned long magnetTime = 0;

int sitDistance = 35;
int standDistance = 80;

unsigned long nr1Time = 45000;

unsigned long minimumTime = 20000;

//states
bool roomEmpty = true;
bool inUse = false;
bool magnet = true;
bool movingState;
bool menuActive = false;

bool magnetize = false;

int motionTimer = 0;
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);


void setup() {
  Serial.begin(9600);
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  for (int i = 0; i < numberOfDevices; i++)
  {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i))
    {
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    }
  }

  lcd.begin(16, 2);

  initializeMenu(&lcd);
  showIdleMenu();
  sprayDelay = EEPROM.read(15) * 1000;
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motionPin, INPUT);
  pinMode(ledPin, OUTPUT); // Use LED indicator (if required)
  pinMode(LDRPin, INPUT);
  pinMode(magnetPin, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Sprays: ");
  amountOfSpraysLeft = readEEPROM();
  lcd.print(amountOfSpraysLeft);
}

void loop() {
  

  movingState = digitalRead(motionPin);
  magnetState = digitalRead(magnetPin);

  unsigned long currentMillis = millis();

  if (magnetState == HIGH && magnetize == false && (currentMillis - magnetTime) > 500){
    magnetize = true;
    magnetTime = currentMillis;
    nr1Timer = currentMillis;
    sprayButTimer = currentMillis;
    lcd.setCursor(13,0);
    lcd.print("ON");
  }
  else if(currentMillis - magnetTime > 30000 && !inUse && !sprayButtonTouched){
    magnetize = false;
    magnetTime = currentMillis;
    lcd.setCursor(13,0);
    lcd.print("  ");
  }


  spraying(currentMillis);

  int reading = digitalRead(magnetPin);

  // If the switch changed, due to noise or pressing:
  if (reading != lastMagnetState) {
    // reset the debouncing timer
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != magnetState) {
      magnetState = reading;
    }
  }

  if (menuButton.read() && currentMillis > 500) {
    activateMenu();
    menuActivatedCount += 1;
    menuActive = true;
    sitAttemptTime = 0;
    standAttemptTime = 0;
    useAttemptTime = 0;
    standTime = 0;
    sitTime = 0;
    useTime = 0;
    menuTimer = currentMillis;
  }
  else {
    if (menuActive) {
      long* rb;
      rb = resetMenu();
      if (rb[0] == 0) {
        menuActive = false;
        sprayDelay = rb[1];
        EEPROM.write(15, sprayDelay / 1000);
        if (rb[2] == 0) {
          resetEEPROM(2500);//eepromWrite(2243);
          amountOfSpraysLeft = readEEPROM();
        }
      }
      lcd.setCursor(0, 0);
      lcd.print("Sprays: ");
      lcd.print(amountOfSpraysLeft);
    }
  }
  if (menuActive && choiceButton.read()) {
    selectMenu();
  }

  // set the LED:
  //digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastMagnetState = reading;

  if (!menuActive) {
    if (roomEmpty && movingState == HIGH) {
      roomEmpty = false;
      digitalWrite(redLED, HIGH);
      redOn = true;
      motionTimer = 0;
    }

    else if (!roomEmpty && !inUse && movingState == LOW) {
      motionTimer += currentMillis - previousMillisMotion; //add ms to timer
      previousMillisMotion = currentMillis;

      if (motionTimer > triggertime) {
        roomEmpty = true;
        digitalWrite(redLED, LOW);
        motionTimer = 0;
      }

    }

    if (inUse) {
      motionTimer = 0;
    }

    if (currentMillis - previousMillisDist >= distanceDelay && !roomEmpty && !sprayButtonTouched) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);

      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);

      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);

      //Calculate the distance (in cm) based on the speed of sound.
      d = duration / 58.2;

      checkDistanceState(d);
      previousMillisDist = currentMillis;
    }

    if (currentMillis - prevTempTime >= tempDelay) {

      sensors.requestTemperatures(); // Send the command to get temperatures

        // Loop through each device, print out temperature data
      for (int i = 0; i < numberOfDevices; i++)
      {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i))
        {
          // Output the device ID

          // It responds almost immediately. Let's print out the data
          printTemperature(tempDeviceAddress); // Use a simple function to print out the data


          float tempC = sensors.getTempC(tempDeviceAddress);
          lcd.setCursor(0, 1);
          lcd.print("Temp C: ");
          lcd.print(tempC);


        }
        //else ghost device! Check your power requirements and cabling

      }

      int light = analogRead(LDRPin);
      prevTempTime = currentMillis;
    }
  }
}

void checkDistanceState(int dist) {
  unsigned long current = millis();
  unsigned long timeDif = current - previousDistState;
  bool far = (dist > standDistance);
  if (!far) {
    fleeAttemptTime = 0;
  }
  previousDistState = current;
  if (current - lastUsed >= usageDelay) {
    //add Magnetic sensor
    if (!inUse) {
      if (!attemptToUse && !far) {
        attemptToUse = true;
        useAttemptTime = 0;
      }
      else if (attemptToUse && !far && useAttemptTime < decisionTime) {
        useAttemptTime += timeDif;
      }
      else if (attemptToUse && !far && useAttemptTime >= decisionTime) {
        inUse = true;
        greenOn = true;
        //digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, LOW);
        useTime = 0;
        standing = true;
        standTime = 0;
        attemptToUse = false;
      }

      else {
        useAttemptTime = 0;
      }
    }



    else if (standing) {
      bool sitClose = (dist <= sitDistance);
      bool standClose = (dist <= standDistance);

      if (current - greenTimer > 500) {
        if (greenOn) {
          digitalWrite(greenLED, LOW);
          greenOn = false;
          digitalWrite(redLED, LOW);
          redOn = false;

        }
        else {
          digitalWrite(greenLED, HIGH);
          greenOn = true;
          digitalWrite(redLED, HIGH);
          redOn = true;
        }
        greenTimer = current;
      }

      if (standClose && !sitClose) {
        standTime += timeDif;
        useTime += timeDif;
      }


      else {
        if (!sitting && sitClose && !attemptToSit) {
          attemptToSit = true;
          sitAttemptTime = 0;
        }
        else if (attemptToSit && sitClose && sitAttemptTime < decisionTime) {
          sitAttemptTime += timeDif;
        }

        else if (attemptToSit && sitClose && sitAttemptTime >= decisionTime) {
          sitting = true;
          maxStandTime = max(maxStandTime, standTime);
          standing = false;
          standTime = 0;
          sitTime = 0;
          attemptToSit = false;
        }
        else {
          sitAttemptTime = 0;
        }
      }

    }

    else if (sitting) {
      bool sitClose = (dist <= sitDistance);
      bool standClose = (dist <= standDistance);

      if (current - greenTimer > 1500) {
        if (greenOn) {
          digitalWrite(greenLED, LOW);
          greenOn = false;
          digitalWrite(redLED, LOW);
          redOn = false;

        }
        else {
          digitalWrite(greenLED, HIGH);
          greenOn = true;
          digitalWrite(redLED, HIGH);
          redOn = true;
        }
        greenTimer = current;
      }

      if (sitClose) {
        sitTime += timeDif;
        useTime += timeDif;

      }

      else {
        if (!sitClose && standClose && !attemptToStand) {
          attemptToStand = true;
          standAttemptTime = 0;
        }
        else if (!sitClose && standClose && attemptToStand && standAttemptTime < decisionTime) {
          standAttemptTime += timeDif;
        }
        else if (!sitClose && standClose && attemptToStand && standAttemptTime >= decisionTime) {
          standing = true;
          sitting = false;
          maxSitTime = max(maxSitTime, sitTime);
          sitTime = 0;
          standTime = 0;
          attemptToStand = false;
        }
        else {
          standAttemptTime = 0;
        }
      }
    }


    if (far && inUse && current - lastUsed >= usageDelay) {
      if (!attemptToFlee && far) {
        attemptToFlee = true;
        fleeAttemptTime = 0;
      }
      else if (attemptToFlee && fleeAttemptTime < decisionTime && far) {
        fleeAttemptTime += timeDif;
      }
      else if (attemptToFlee && fleeAttemptTime >= decisionTime && far) {
        inUse = false;

        if (standing) {
          maxStandTime = max(maxStandTime, standTime);
        }
        if (sitting) {
          maxSitTime = max(maxSitTime, sitTime);
        }
        standing = false;
        sitTime = 0;
        standTime = 0;
        sitting = false;
        maxUseTime = max(maxUseTime, useTime);
        useTime = 0;
        lastUsed = current;
        digitalWrite(redLED, LOW);

        usageDone(maxUseTime, maxStandTime, maxSitTime);
        maxUseTime = 0;
        maxStandTime = 0;
        maxSitTime = 0;
      }
    }

  }





}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

void usageDone(unsigned long maxUse, unsigned long maxStand, unsigned long maxSit) {
  //lcd.clear();
  //lcd.setCursor(0, 0);

  if (maxUse < minimumTime) {
    //lcd.print("Nothing");
    digitalWrite(greenLED, LOW);
  }
  else if (maxSit <= nr1Time) {
    //lcd.print("Spray: 1 time");
    nr1Done = true;
    nr1Timer = millis();
    sprayButtonTouched = true;
    sprayButTimer = millis();
  }
  else if (maxSit > nr1Time) {
    //lcd.print("Spray: 2 times");
    nr2Done = true;
    nr2Timer = millis();
    sprayButtonTouched = true;
    sprayButTimer = millis();
  }


}

void spraying(long currentMillis) {
  if (sprayButton.read() && currentMillis > 500) {
    //lcd.clear();
    //lcd.setCursor(0, 0);
    //lcd.print("Spraying in");
    nr1Done = true;
    nr1Timer = currentMillis;
    sprayButtonTouched = true;
    magnetize = true;
    sprayButTimer = currentMillis;
  }
  if(currentMillis - magnetTime < 10000 && magnetize && nr1Done){
    sprayButtonTouched = true;
  }

  if (currentMillis - sprayButTimer > sprayDelay) {
    sprayButtonTouched = false;
  }
  else if (sprayButtonTouched && magnetize) {
    digitalWrite(greenLED, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Spraying in  ");
    lcd.setCursor(0, 1);
    lcd.print("seconds: ");
    lcd.print("~");
    lcd.print((sprayDelay  - (currentMillis - sprayButTimer)) / 1000);
    lcd.print("  ");
  }
  if (nr1Done && currentMillis - nr1Timer > sprayDelay && magnetize) {
    //digitalWrite(greenLED, HIGH);
  }

  if (nr1Done && currentMillis - nr1Timer > sprayDelay - ledDelay && magnetize) {
    digitalWrite(ledPin, HIGH);
    nr1Done = false;
    nr1Light = true;
  }

  if ((nr1Light && currentMillis - nr1Timer > sprayDelay + extraDelay) || (nr2Light && currentMillis - nr2Timer > sprayDelay + extraDelay && magnetize)) {
    digitalWrite(ledPin, LOW);
    if (nr2Light) {
      nr1Done = true;
      nr2Light = false;
      nr1Timer = currentMillis;
    }
    else {
      nr1Light = false;
      digitalWrite(greenLED, LOW);
    }

    amountOfSpraysLeft = readEEPROM() - 1;
    writeEEPROM(amountOfSpraysLeft);

    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sprays: ");
    lcd.print(amountOfSpraysLeft);
    digitalWrite(redLED, LOW);
    magnetize = false;
    lcd.setCursor(13,0);
    lcd.print("  ");

  }

  if (nr2Done && currentMillis - nr2Timer > sprayDelay - ledDelay) {
    nr2Light = true;
    nr2Done = false;
    digitalWrite(ledPin, HIGH);
  }

}


void writeEEPROM(int value){

  int i = EEPROM.read(11);
  int curval = EEPROM.read(i);
  if (curval == 1){
    EEPROM.write(11, i-1);
    writeEEPROM(value);
  }
  else{
    EEPROM.write(i, curval-1);
  }
}

void resetEEPROM(int value){

  for (int n=0; n<10; n++){
    if (value <= 255){
      EEPROM.write(n, value);
    }
    else{
      EEPROM.write(n, 255);
    }
    value-= 255;
  }
  EEPROM.write(11, 9);
}

int readEEPROM(){
  int value = 0;
  int i = EEPROM.read(11);
  for (int n=0; n<=i; n++){
    value += EEPROM.read(n);
  }
  return value;
}

