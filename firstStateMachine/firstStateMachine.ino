#define echoPin 9 // Echo Pin
#define trigPin 10 // Trigger Pin
#define LEDPin 13 // Onboard LED
#define motionPin 8 // Motion Pin

int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
unsigned long previousMillisMotion = 0;
unsigned long previousMillisDist = 0
unsigned long triggerTime = 2000; // Set time (ms) you need to have no motion to change state 
unsigned long distanceDelay = 500; // Set time (ms) interval when distance is updated (on LCD)

bool roomEmpty = true;
bool inUse = false;

int motionTimer = 0;


#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup() {
 lcd.begin(16, 2);
 lcd.print("Room Empty");
 lcd.setCursor(0, 1);
 lcd.print("Not in Use");

 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(motion, INPUT);
 pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
}

void loop() {
 movingState = digitalRead(motionPin);
 unsigned long currentMillis = millis()
 /* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */
 
 if (roomEmpty && movingState == HIGH){
   lcd.setCursor(0,0);
   lcd.print("Room inside");
   roomEmpty = false;
 }
 
 else if (!roomEmpty && movingState == LOW){
   motionTimer += currentMillis - previousMillisMotion; //add ms to timer
   previousMillisMotion = currentMillis;
   
   if (motionTimer > triggerTime){
     roomEmpty = true;
     lcd.setCursor(0,0);
     lcd.print("              ");
     lcd.setCursor(0,0);
     lcd.print("Room empty");
     motionTimer = 0;
   }
 }
   
 
 if (currentMillis - previousMillisDist >= distanceDelay){
   digitalWrite(trigPin, LOW); 
   delayMicroseconds(2); 
  
   digitalWrite(trigPin, HIGH);
   delayMicroseconds(10); 
   
   digitalWrite(trigPin, LOW);
   duration = pulseIn(echoPin, HIGH);
   
   //Calculate the distance (in cm) based on the speed of sound.
   distance = duration/58.2;
   
   if (distance >= maximumRange || distance <= minimumRange){
     lcd.clear();
     lcd.print("Out of range");
   }
   else {
     lcd.clear();
     lcd.print("Distance:");
     lcd.setCursor(0, 1);
     lcd.print(distance);
   }
   previousMillisDist = currentMillis;
 }
 
  
  
}




