#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


#define echoPin 4// Echo Pin
#define trigPin 3// Trigger Pin
#define LEDPin 13 // Onboard LED
#define motionPin 6 // Motion Pin

int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, d; // Duration used to calculate distance
unsigned long previousMillisMotion = 0;
unsigned long previousMillisDist = 0;
unsigned long prevTempTime = 0;
unsigned long tempDelay = 3000;
unsigned long triggertime = 2000; // Set time (ms) you need to have no motion to change state 
unsigned long distanceDelay = 250;
int minDistance = 25;

unsigned long decisionTime = 1000;

//Distance helper variables
bool sitting = false;
bool standing = false;
bool attemptToSit = false;
bool attemptToStand = false;
bool attemptToUse = false;
bool attemptToFlee = false;

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


//states
bool roomEmpty = true;
bool inUse = false;
bool magnet = true;
bool movingState;


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
		Serial.print("Found device ");
		Serial.print(i, DEC);
		Serial.print(" with address: ");
		//printAddress(tempDeviceAddress);
		Serial.println();
		
		Serial.print("Setting resolution to ");
		Serial.println(TEMPERATURE_PRECISION, DEC);
		
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
		
		 Serial.print("Resolution actually set to: ");
		Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
		Serial.println();
	}else{
		Serial.print("Found ghost device at ");
		Serial.print(i, DEC);
		Serial.print(" but could not detect address. Check power and cabling");
	}
  }
  
 lcd.begin(16, 2);
 lcd.print("Room Empty");
 lcd.setCursor(0, 1);
 lcd.print("Not in Use");
 
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(motionPin, INPUT);
 pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
}

void loop() {
 movingState = digitalRead(motionPin);
 unsigned long currentMillis = millis();
 
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
     lcd.print("              ");
     lcd.setCursor(0,0);
     lcd.print("Room empty");
     motionTimer = 0;
   }
 }
 
 if (inUse){
   motionTimer = 0;
   previousMillisMotion = currentMillis;
 }
 
 if (currentMillis - previousMillisDist >= distanceDelay){ //&& !roomEmpty){
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
	} 
	//else ghost device! Check your power requirements and cabling
	
  }
  
  prevTempTime = currentMillis;
 }
}

void checkDistanceState(int dist){
  unsigned long current = millis();
  unsigned long timeDif = current - previousDistState;
  bool far = (dist > standDistance);
  previousDistState = current;
  
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
  
  
  if (far){
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





