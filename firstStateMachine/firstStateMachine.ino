#include <OneWire.h>
#include <DallasTemperature.h>
#include <Menu.h>
#include <Button.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
Button menuButton(17); //A3
Button choiceButton(18); //A4
Button sprayButton(19); // A5
int amountOfSpraysLeft = 1250;

#define echoPin 4// Echo Pin
#define trigPin 3// Trigger Pin
#define ledPin 13 // Onboard LED
#define motionPin 5 // Motion Pin
#define LDRPin 0 // Light Sensor
#define magnetPin 15 // Magnetic sensor

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
unsigned long usageDelay = 15000;
unsigned long lastUsed = -usageDelay;
unsigned long ledDelay = 15000;
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

int sitDistance = 10;
int standDistance = 35;

unsigned long nr1Time = 45000;
unsigned long minimumTime = 20000;

//states
bool roomEmpty = true;
bool inUse = false;
bool magnet = true;
bool movingState;
bool menuActive = false;


int motionTimer = 0;
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);


void setup() {
  Serial.begin(9600);
  sensors.begin();
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{
		sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
	}
  }
  
 lcd.begin(16, 2);

 initializeMenu(&lcd);
 showIdleMenu();
 
 lcd.setCursor(0,0);
 lcd.print("Room Empty");
 lcd.setCursor(0, 1);
 lcd.print("Not in Use");
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(motionPin, INPUT);
 pinMode(ledPin, OUTPUT); // Use LED indicator (if required)
 pinMode(LDRPin, INPUT);
 pinMode(magnetPin, INPUT);
}

void loop() {
 movingState = digitalRead(motionPin);
 //magnetState = digitalRead(magnetPin);
 
 unsigned long currentMillis = millis();
 
 
 
 if (sprayButton.read() && currentMillis > decisionTime) {
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Spraying in");
   nr1Done = true;
   nr1Timer = currentMillis;
   sprayButtonTouched = true;
   sprayButTimer = currentMillis;
 }
 
 if (currentMillis - sprayButTimer > sprayDelay){
   sprayButtonTouched = false;
 }
 else if (sprayButtonTouched){
   lcd.setCursor(0,0);
   lcd.print("Spraying in");
   lcd.setCursor(0,1);
   lcd.print("seconds: ");
   lcd.print("~");
   lcd.print((sprayDelay  - (currentMillis - sprayButTimer))/ 1000);
   lcd.print("  ");
 }
 
 if (nr1Done && currentMillis - nr1Timer > sprayDelay - ledDelay){
   digitalWrite(ledPin, HIGH);
   Serial.println("Light 1 on");
   nr1Done = false;
   nr1Light = true;
 }
 
 if ((nr1Light && currentMillis - nr1Timer > sprayDelay) || (nr2Light && currentMillis - nr2Timer > sprayDelay)){
   digitalWrite(ledPin, LOW);
   if (nr2Light){
     nr1Done = true;
     nr2Light = false;
     Serial.println("Light 2 off");
     nr1Timer = currentMillis;
   }
   else {
     nr1Light = false;
     Serial.println("Light 1 off");
   }
   
   amountOfSpraysLeft -= 1;
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Sprays left:");
   lcd.setCursor(0,1);
   lcd.print(amountOfSpraysLeft);
 }
 
 if (nr2Done && currentMillis - nr2Timer > sprayDelay - ledDelay){
   nr2Light = true;
   Serial.println("Light 2 on");
   nr2Done = false;
   digitalWrite(ledPin, HIGH);
 }
 
  int reading = digitalRead(magnetPin);
  //Serial.print(reading);
  //Serial.println();
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

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
      Serial.print("Change MAGNET");
      Serial.println();
      magnetState = reading;
     // ledState = !ledState;
      
    }
  }
  
  if (menuButton.read() && currentMillis > decisionTime){
    Serial.print("Activated Menu");
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
    if (menuActive){
      long* rb;
      rb = resetMenu();
      if (rb[0] == 0){
         menuActive = false;
         sprayDelay = rb[1];
         if (rb[2] == 1){
           amountOfSpraysLeft = 1250;
         }
      }        
    }
  }
  if (menuActive && choiceButton.read()){
    selectMenu();
  }

  // set the LED:
  //digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastMagnetState = reading;
  
 
 if (roomEmpty && movingState == HIGH){
   lcd.setCursor(0,0);
   lcd.print("Room inside");
   roomEmpty = false;
   motionTimer = 0;
 }
 
 else if (!roomEmpty && !inUse && movingState == LOW){
   motionTimer += currentMillis - previousMillisMotion; //add ms to timer
   previousMillisMotion = currentMillis;
   
   if (motionTimer > triggertime){
     roomEmpty = true;
     lcd.setCursor(0,0);
     lcd.print("           ");
     lcd.setCursor(0,0);
     lcd.print("Room empty");
     motionTimer = 0;
   }
 }
 
 if (inUse){
   motionTimer = 0;
 }
 
 if (!menuActive && currentMillis - previousMillisDist >= distanceDelay && !roomEmpty && !sprayButtonTouched){
       digitalWrite(trigPin, LOW); 
       delayMicroseconds(2); 
      
       digitalWrite(trigPin, HIGH);
       delayMicroseconds(10); 
       
       digitalWrite(trigPin, LOW);
       duration = pulseIn(echoPin, HIGH);
       
       //Calculate the distance (in cm) based on the speed of sound.
       d = duration/58.2;
     
       checkDistanceState(d);
       previousMillisDist = currentMillis;
 }
 
 if (currentMillis - prevTempTime >= tempDelay){
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{
		// Output the device ID
		Serial.print("Temperature for device: ");
		Serial.println(i,DEC);
		
		// It responds almost immediately. Let's print out the data
		printTemperature(tempDeviceAddress); // Use a simple function to print out the data
                if (currentMillis - lastUsed < usageDelay){
                  //lcd.clear();
                  float tempC = sensors.getTempC(tempDeviceAddress);
                  //lcd.print("Temp C:");
                  //lcd.print(tempC);
                  //lcd.setCursor(0,1);
                  //lcd.print("Empty: ");
                  //lcd.print(roomEmpty);
                  Serial.print("On LCD");
                  Serial.println();
                  }
               
        }
	//else ghost device! Check your power requirements and cabling
	
  }
  
  int light = analogRead(LDRPin);
  Serial.println("The current light =:");
  Serial.println(light);
  prevTempTime = currentMillis;
 }
}

void checkDistanceState(int dist){
  unsigned long current = millis();
  unsigned long timeDif = current - previousDistState;
  bool far = (dist > standDistance);
  if (!far){
    fleeAttemptTime = 0;
  }
  previousDistState = current;
  if (current - lastUsed >= usageDelay){
    if (!inUse){
      if (!attemptToUse && !far){
        attemptToUse = true;
        useAttemptTime = 0;
      }
      else if (attemptToUse && !far && useAttemptTime < decisionTime){
       useAttemptTime += timeDif; 
      }
      else if (attemptToUse && !far && useAttemptTime >= decisionTime){
        inUse = true;
        useTime = 0;
        standing = true;
        standTime = 0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("In Use");
        lcd.setCursor(0,1);
        lcd.print("Standing");
        attemptToUse = false;
      }
      
      else{
        useAttemptTime = 0;
      }
    }
    
    
    
    else if (standing){
      bool sitClose = (dist <= sitDistance);
      bool standClose = (dist <= standDistance);
      
      if (standClose && !sitClose){
        standTime += timeDif;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(dist);
        lcd.setCursor(0,1);
        lcd.print("Standing ");
        lcd.print(standTime / 1000);
        useTime += timeDif;
      }
      
      else {
        if (!sitting && sitClose && !attemptToSit){
          attemptToSit = true;
          sitAttemptTime = 0;
        }
        else if (attemptToSit && sitClose && sitAttemptTime < decisionTime){
          sitAttemptTime += timeDif;
        }
        
        else if (attemptToSit && sitClose && sitAttemptTime >= decisionTime){
          sitting = true;
          lcd.clear();
          lcd.setCursor(0,0);
          //lcd.print("In Use");
          lcd.setCursor(0,1);
          lcd.print("Sitting");
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
    
    else if (sitting){
      bool sitClose = (dist <= sitDistance);
      bool standClose = (dist <= standDistance);
      
      if (sitClose){
        sitTime += timeDif;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(timeDif);
        lcd.setCursor(0,1);
        lcd.print("Sitting ");
        lcd.print(sitTime / 1000);
        useTime += timeDif;
      }
      
      else {
        if (!sitClose && standClose && !attemptToStand){
          attemptToStand = true;
          standAttemptTime = 0;
        }
        else if (!sitClose && standClose && attemptToStand && standAttemptTime < decisionTime){
          standAttemptTime += timeDif;
        }
        else if (!sitClose && standClose && attemptToStand && standAttemptTime >= decisionTime){
          standing = true;
          sitting = false;
          lcd.setCursor(0,1);
          lcd.print("Standing");
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
    
    
    if (far && inUse && current - lastUsed >= usageDelay){
      if (!attemptToFlee && far){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("fleeing");
        lcd.setCursor(0,1);
        if (standing){
          lcd.print("Standing");
        }
        else {
          lcd.print("Sitting");
        }
        attemptToFlee = true;
        fleeAttemptTime = 0;
      }
      else if (attemptToFlee && fleeAttemptTime < decisionTime && far){
        fleeAttemptTime += timeDif;
      }
      else if (attemptToFlee && fleeAttemptTime >= decisionTime && far){
        inUse = false;
        
        if (standing){
          maxStandTime = max(maxStandTime, standTime); 
        }
        if (sitting){
          maxSitTime = max(maxSitTime, sitTime);
        }
        standing = false;
        sitTime = 0;
        standTime = 0;
        sitting = false;
        maxUseTime = max(maxUseTime, useTime);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Max use: ");
        lcd.print(maxUseTime / 1000);
        lcd.setCursor(0,1);
        if (maxSitTime > 0){
          lcd.print("Sit for ");
          lcd.print(maxSitTime / 1000);
          lcd.print("s");
        }
        else {
          lcd.print("Stood for ");
          lcd.print(maxStandTime / 1000);
          lcd.print("s");
        }
        useTime = 0;
        lastUsed = current;
        Serial.println("Usage done...");
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

void usageDone(unsigned long maxUse, unsigned long maxStand, unsigned long maxSit){
  Serial.print("checking usage...");
  Serial.print("Max Use: ");
  Serial.print(maxUse);
  Serial.println();
  Serial.print("Max Stand: ");
  Serial.print(maxStand);
  Serial.println();
  Serial.print("Max Sit: ");
  Serial.print(maxSit);  
  lcd.clear();
  lcd.setCursor(0,0);
  
  if (maxUse < minimumTime){
    Serial.println("Not used");
    lcd.print("Nothing");
  }
  else if (maxUse <= nr1Time){
    Serial.println("Nr 1 done");
    Serial.println("Spray once");
    lcd.print("Spray: 1 time");
    nr1Done = true;
    nr1Timer = millis();
  }
  else if (maxUse > nr1Time){
    Serial.println("Nr 2 done");
    Serial.println("Spray twice");
    lcd.print("Spray: 2 times");
    nr2Done = true;
    nr2Timer = millis();
  }
    

}


