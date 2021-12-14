#include <Arduino.h>
#include <WString.h>
#include "EspressoController.h"

typedef void (*messagePtr)(String);// Create a type to point to a function.

class Tank
{
    // Class Member Variables
    // These are initialized at startup
    int sensorPin;      // the number of the Sensor pin
    int relayPin;      // the number of the Relay pin to operate the fill valve solenoid

    // These maintain the current state
    
    unsigned long previousTime;   // will store last time LED was updated
    
    messagePtr message; // Create an instance of the empty function pointer
    espressoState *state;
    
  public:
    bool full;                 // if the tank needs filling
    bool filling;
    unsigned long updateTime;     // milliseconds till update
    unsigned long delayTime;     // milliseconds till update

    Tank(int _sensorpin,
         int _relaypin,
         unsigned long _updateTime,
         unsigned long _delayTime,
         messagePtr _message,
         espressoState *_state) {
          
      sensorPin = _sensorpin;
      pinMode(sensorPin, INPUT);

      relayPin = _relaypin;
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, HIGH);//turn relay off

      updateTime = _updateTime;
      delayTime = _delayTime;
      previousTime = 0;
      full = false;
      filling = false;
      state = _state;
      message = _message;
    }

    void update()
    {
      // check to see if it's time to check the water level
      unsigned long currentMillis = millis();
      unsigned long thisUpdateTime = updateTime;
      
      if (filling) {
        thisUpdateTime = 1000;//reduce update time to 1 second when pump is on and filling tank.
      }

      if (currentMillis - previousTime >= thisUpdateTime) {
        previousTime = currentMillis;  // Remember the time
        if (sensorFull()) {
          full = true;
          digitalWrite(relayPin, HIGH);//turn relay off since we are full - dont want to overfill
          message(F("TANK FULL"));
          message(F("CLOSING TANK SOLENOID"));
        } else {
          full = false;
          message(F("TANK LOW"));
          //We open the valve once we know the pump is on - see below.
        }
      }
    }
    
    bool sensorFull(){
      //message(F("sensing Tank Fullness"));
      digitalWrite(sensorPin, HIGH);//write HIGH instead of PULLUP Permanently to reduce electrolysis
      int levelPinValue = digitalRead(sensorPin);
      digitalWrite(sensorPin, LOW);//return to low to prevent electrolysis
      if (levelPinValue == 0){
        return true;
      }else{
        return false; 
      }
    }
    void pumpOn(){
      filling = true;
      if (!full){ //only fill is no full already
        digitalWrite(relayPin, LOW);//turn relay on so tank can fill up
        message(F("OPENING TANK SOLENOID"));
      }
    }
    void pumpOff(){
      filling = false;
    }
    
//    void unPause() {
//      pausedCount--; //we only unPause when Zero so multiple functions can request a Pause and we know when they are all done.
//      if (pausedCount == 0){
//        paused = false;
//      }
//      message("TANK PAUSE COUNT: " + StringSumHelper(pausedCount));
//    }
//    void pause() {
//      pausedCount++; //add one to pausedCount - we only unPause when Zero so multiple functions can request a Pause.
//      paused = true;
//      filling = false;
//      digitalWrite(relayPin, HIGH);//turn relay off
//      message("TANK PAUSE COUNT: " + StringSumHelper(pausedCount));
//    }
};
