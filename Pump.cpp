#include <Arduino.h>
#include <WString.h>
#include "EspressoController.h"

typedef void (*messagePtr)(String);// Create a type to point to a function.

struct pumpStates {
  bool groupLEFT = false;
  bool groupRIGHT = false;
  bool tank = false;
  
  bool isOff() {
    if(!groupLEFT && !groupRIGHT && !tank){
      return true;
    }else{
      return false;
    }
  }
  bool isOn() {
    if(groupLEFT || groupRIGHT || tank){
      return true;
    }else{
      return false;
    }
  }
  setState(String component, bool state){
    if (component == "groupLEFT"){
       groupLEFT = state;
    }else if (component == "groupRIGHT"){
       groupRIGHT = state;
    }else if (component == "tank"){
       tank = state;
    }
  }
};

class Pump
{
    // Class Member Variables
    // These are initialized at startup
    int relayPin;      // the number of the Relay pin to operate the pump

    messagePtr message; // Create an instance of the empty function pointer
    espressoState *state;
    pumpStates pumpState;
    
    // Constructor - creates a Pump Object
    // and initializes the member variables and state
  public:
    bool stateChanged = false;                 // if the pump is on
 
    Pump(int _relaypin,
         messagePtr _message,
         espressoState *_state) {
          
      relayPin = _relaypin;
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, HIGH);//turn pump relay off

      state = _state;
      message = _message;
    }

    void update()
    {
      if (pumpState.isOn() && stateChanged){ //if we have 1 or more requests for the pump to be on turn it on. 
        digitalWrite(relayPin, LOW);//turn relay on
        message(F("TURNING PUMP ON"));
        stateChanged = false;
      }else if (pumpState.isOff() && stateChanged){//turn it off
        digitalWrite(relayPin, HIGH);//turn relay off
        message(F("TURNING PUMP OFF"));
        stateChanged = false;
      }
    }
    
    bool pumpOnFor(String component){
      if (component == "groupLEFT"){
         return pumpState.groupLEFT;
      }else if (component == "groupRIGHT"){
         return pumpState.groupRIGHT;
      }else if (component == "tank"){
         return pumpState.tank;
      }
    }
    
    void pumpOff(String component) {
      pumpState.setState(component, false);
      message("PUMP USE SET FALSE FOR:" + component);
      stateChanged = true;
    }
    
    void pumpOn(String component) {
      pumpState.setState(component, true);
      message("PUMP USE SET TRUE FOR:" + component);
      stateChanged = true;
    }

};
