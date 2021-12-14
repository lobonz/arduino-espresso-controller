#include <Arduino.h>
#include <WString.h>
#include "src/Button-Lobo/src/Button.h"
#include "src/Single_LED_Library/singleLEDLibrary.h"
#include "EspressoController.h"

//https://stackoverflow.com/questions/44021219/arduino-pass-function-to-class-returning-string
typedef void (*messagePtr)(String);// Create a type to point to a function.
typedef void (*resetFlowmeterPtr)();// Create a type to point to a function.
typedef unsigned long (*readFlowmeterPtr)();// Create a type to point to a function.
typedef void (*updateAndSaveSettingsPtr)();// Create a type to point to a function.

class Group
{
    // Class Member Variables
    // These are initialized at startup
    int groupPin; // the number of the LED pin
    
    int buttonPin_ST;
    int ledPin_ST;
    int buttonPin_GO;
    int ledPin_GO;
    int buttonPin_SS;
    int ledPin_SS;
    int buttonPin_DS;
    int ledPin_SD;
    espressoState *state;
    
    messagePtr message; // Create an instance of the empty function pointer
    resetFlowmeterPtr resetFlowmeter;
    readFlowmeterPtr readFlowmeter;
    updateAndSaveSettingsPtr updateAndSaveSettings;
    
    unsigned long lastFlowCheckTime;
    int lastPulses;
  public:
    int pulsesSS; //pulses for a single shot
    int pulsesDS; //pulses for a double shot

    // These maintain the current state
    bool extracting;
    bool extractingSS;
    bool extractingDS;
    bool programmingMode;
    bool programmingExtraction;
    bool programmingSS;
    bool programmingDS;
    
    //Button LEDs
    sllib led_ST;
    sllib led_GO;
    sllib led_SS;
    sllib led_DS;

    //Buttons
    Button button_ST;
    Button button_GO;
    Button button_SS;
    Button button_DS;

    Group(int _groupPin,
          unsigned long _pulsesSS,
          unsigned long _pulsesDS,
          int _buttonPin_ST,
          int _ledPin_ST,
          int _buttonPin_GO,
          int _ledPin_GO,
          int _buttonPin_SS,
          int _ledPin_SS,
          int _buttonPin_DS,
          int _ledPin_DS,
          messagePtr _message,
          resetFlowmeterPtr _resetFlowmeter,
          readFlowmeterPtr _readFlowmeter,
          updateAndSaveSettingsPtr _updateAndSaveSettings,
          espressoState *_state
         ) {

      groupPin = _groupPin;
      pinMode(groupPin, OUTPUT);
      digitalWrite(groupPin, HIGH);//turn relay off
      
      programmingMode = false;
      programmingSS = false;
      programmingDS = false;
      programmingExtraction = false;

      led_ST.begin(_ledPin_ST);
      led_GO.begin(_ledPin_GO);
      led_SS.begin(_ledPin_SS);
      led_DS.begin(_ledPin_DS);

      // Create Buttons
      button_ST.begin(_buttonPin_ST, B_PULLUP, 5000);//long press to program on stop button
      button_GO.begin(_buttonPin_GO, B_PULLUP, 0);
      button_SS.begin(_buttonPin_SS, B_PULLUP, 0);
      button_DS.begin(_buttonPin_DS, B_PULLUP, 0);

      pulsesSS = _pulsesSS; //pulses for a single shot
      pulsesDS = _pulsesDS; //pulses for a double shot
    
      message = _message;
      resetFlowmeter = _resetFlowmeter;
      readFlowmeter = _readFlowmeter;
      updateAndSaveSettings = _updateAndSaveSettings;
      state = _state;
      lastFlowCheckTime = millis();
      lastPulses = 0;
    }

    void update() {
      led_ST.update();
      led_GO.update();
      led_SS.update();
      led_DS.update();

      /////////////////////////////////////
      //  UPDATE BUTTONS AND HANDLE EVENTS
      /////////////////////////////////////
      handleEventST(button_ST.update());//send the button state to the  button handler
      handleEventGO(button_GO.update());//send the button state to the  button handler
      handleEventSS(button_SS.update());//send the button state to the  button handler
      handleEventDS(button_DS.update());//send the button state to the  button handler

      if (extracting && (extractingSS || extractingDS)){ //need to check on flow meter and stop when required
        if ( millis() - lastFlowCheckTime > state->flowCheckTime){
          lastFlowCheckTime = millis();
          unsigned long  pulses = readFlowmeter();
          if (lastPulses != pulses){
            message("CHECKING PULSES: " + StringSumHelper(pulses));
            lastPulses = pulses;
          }
          if (extractingSS && pulses >= pulsesSS){
            stopExtracting();
            message(F("COMPLETED SS EXTRACTION"));
          }else if (extractingDS && pulses >= pulsesDS){//extractingDS
            stopExtracting();
            message(F("COMPLETED DS EXTRACTION"));
          }
        }
      }else if (extracting){
        if (millis() - lastFlowCheckTime > state->extractionTimeout){//need to stop extracting if we extract longer than timeout.
          stopExtracting();
          message(F("STOPPED EXTRACTION DUE TO TIMEOUT"));
        }
      }
    }
    
    void beginProgrammingMode() {
      programmingMode = true;
    }

    void beginProgrammingExtraction() {
      programmingExtraction = true;
    }

    void beginProgrammingSS() {
      programmingSS = true;
    }

    void beginProgrammingDS() {
      programmingDS = true;
    }

    void exitProgrammingMode() {
      programmingMode = false;
      programmingSS = false;
      programmingDS = false;
      programmingExtraction = false;
      led_ST.setOffSingle();
      led_SS.setOffSingle();
      led_DS.setOffSingle();
      message("EXITING PROGRAMMING MODE FOR GH");
    }

    void startExtracting(){
      lastFlowCheckTime = millis();
      extracting = true;
      resetFlowmeter();
      lastPulses = 0;
      digitalWrite(groupPin, LOW);//turn relay on
    }
    
    void stopExtracting(){
      led_ST.setOffSingle();
      extracting = false;
      led_GO.setOffSingle();
      extractingSS = false;
      led_SS.setOffSingle();
      extractingDS = false;
      led_DS.setOffSingle();
      digitalWrite(groupPin, HIGH);//turn relay off
    }
    ///////////////////////////////////////////////////////////
    ////  BUTTON ACTIONS
    ///////////////////////////////////////////////////////////
  
    void handleEventST(String buttonEvent) {
      if (buttonEvent == "pressed") {
        message("ST PRESSED");
        if (!programmingMode) {
          led_ST.setOnSingle();
        }
      } else if (buttonEvent == "released") {
        message(F("ST RELEASED"));
        if (!programmingMode) {
          led_ST.setOffSingle();
          if(extracting){
             stopExtracting();
          }
        } else if (programmingExtraction) {
          led_ST.setOffSingle();
          if (programmingSS ) {
            led_SS.setOffSingle();
          } else if (programmingMode && programmingDS ) {
            led_DS.setOffSingle();
          }
          led_GO.setOffSingle();

          if (programmingSS) {

            unsigned long  newPulses = readFlowmeter();
            pulsesSS = newPulses;
            updateAndSaveSettings();

            message(F("--- Pulses Read ---"));
            message(StringSumHelper(newPulses));
            message(F("FINISHED PROGRAMMING GH SINGLE SHOT"));

          } else if (programmingDS) {

            unsigned long  newPulses = readFlowmeter();
            pulsesDS = newPulses;
            updateAndSaveSettings();

            message(F("--- Pulses Read ---"));
            message(StringSumHelper(newPulses));
            message(F("FINISHED PROGRAMMING GH DOUBLE SHOT"));
          }

          exitProgrammingMode();
        }
      } else if (buttonEvent ==  "longpressed") {
        message(F("ST LONG PRESSED"));
        //Ignore if already programming the Right Group Head
        if (!programmingMode) {
          //exit programming mode if already programming this GRoup Head
          if (programmingMode) {
            exitProgrammingMode();
          } else { //enter programming mode for this group head
            beginProgrammingMode();
            led_ST.setOnSingle();
            led_SS.setBlinkSingle(500);
            led_DS.setBlinkSingle(500);
            message(F("ENTRING PROGRAMMING MODE FOR GH"));
          }
        }
      }
    }
    void handleEventGO(String buttonEvent) {
      if (buttonEvent == "pressed") {
        message(F("GO PRESSED"));
        if (!programmingMode && !extracting) {
          led_GO.setOnSingle();
        }
      }
      else if (buttonEvent == "released") {
        message(F("GO RELEASED"));
        if (!programmingMode && !extracting) {
          //led_GO.setOffSingle();
          startExtracting();
          led_ST.setBlinkSingle(500);
        } else if (programmingMode) {
          led_GO.setOffSingle();
          led_GO.setOnSingle();
          led_ST.setBlinkSingle(500);
          resetFlowmeter();
          beginProgrammingExtraction();
        }
      }
    }
    void handleEventSS(String buttonEvent) {
      if (buttonEvent == "pressed") {
        message("SS PRESSED");
        if (!programmingMode && !extracting) {
          led_SS.setOnSingle();
        }
      }
      else if (buttonEvent == "released") {
        message("SS RELEASED");
        if (!programmingMode && !extracting) {
//          led_SS.setOffSingle();
           startExtracting();
           extractingSS = true;
           led_SS.setBlinkSingle(500);
        } else if (!programmingSS && programmingMode) {
          led_SS.setOffSingle();
          led_SS.setOnSingle();
          led_DS.setOffSingle();
          led_GO.setBlinkSingle(500);
          beginProgrammingSS();
        }
      }
    }
    void handleEventDS(String buttonEvent) {
      if (buttonEvent == "pressed") {
        message("DS PRESSED");
        if (!programmingMode  && !extracting) {
          led_DS.setOnSingle();
        }
      }
      else if (buttonEvent == "released") {
        message("DS RELEASED");
        if (!programmingMode && !extracting) {
//          led_DS.setOffSingle();
           startExtracting();
           extractingDS = true;
           led_DS.setBlinkSingle(500);
        } else if (!programmingDS && programmingMode) {
          led_DS.setOffSingle();
          led_DS.setOnSingle();
          led_SS.setOffSingle();
          led_GO.setBlinkSingle(500);
          beginProgrammingDS();
        }
      }
    }
};
