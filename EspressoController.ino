#include <Arduino.h>
#include <WString.h>
#include "EspressoController.h"
#include "ArduinoJson.h"

///////////////////////////////////////////////
////    DEBUG - Comment out to stop verbose serial output
///////////////////////////////////////////////
#define DEBUG

///////////////////////////////////////////////
////    RESET FUNCTION
///////////////////////////////////////////////
void(* resetFunc) (void) = 0; //declare reset function @ address 0

///////////////////////////////////////////////
////    CLASSES
///////////////////////////////////////////////
#include "Pump.cpp"
#include "Tank.cpp"
#include "Group.cpp"

unsigned long periodicMessage = 5000;
unsigned long lastMessage;
unsigned long messageCountTime = 0;
/////////////////////////////
//FLOW SENSOR SETUP
/////////////////////////////
int flowSensorPin_LGH = 21; //INT[0] -Interrupt pin required for Flow sensor input
int flowSensorPin_RGH = 20; //INT[0] -Interrupt pin required for Flow sensor input

volatile unsigned long flowSensorPulses_LGH;//This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.
volatile unsigned long flowSensorPulses_RGH;//This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.

///////////////////////////////////////////////////////////////////
///       SETTINGS
///////////////////////////////////////////////////////////////////
struct espressoSettings {
  unsigned long pulsesSS_LGH;
  unsigned long pulsesDS_LGH;

  unsigned long pulsesSS_RGH;
  unsigned long pulsesDS_RGH;

  unsigned long tankUpdateTime;
  unsigned long tankDelayTime;

  //unsigned int checksum;

  void Reset() {
    pulsesSS_LGH = 250;
    pulsesDS_LGH = 350;

    pulsesSS_RGH = 250;
    pulsesDS_RGH = 350;

    tankUpdateTime = 120000; //2 minutes
    tankDelayTime = 180000;//3 minutes
  }
};
#include "EEPROMStore.h"
EEPROMStore<espressoSettings> settings;

///////////////////////////////////////////////
////    COMMAND HANDLER
///////////////////////////////////////////////
#include "CommandHandler.h"
CommandHandler<6> SerialCommandHandler;

/* 'state' has to be globally available, so declare it here */
espressoState state;

// LEFT GH Buttons
const int buttonPin_LST = 22;     // the number of the pushbutton pin
const int ledPin_LST =  23;      // the number of the LED pin

const int buttonPin_LGO = 24;     // the number of the pushbutton pin
const int ledPin_LGO =  25;      // the number of the LED pin

const int buttonPin_LSS = 26;     // the number of the pushbutton pin
const int ledPin_LSS =  27;      // the number of the LED pin

const int buttonPin_LDS = 28;     // the number of the pushbutton pin
const int ledPin_LDS =  29;      // the number of the LED pin


//RIGHT GH Buttons

const int buttonPin_RST = 30;     // the number of the pushbutton pin
const int ledPin_RST =  31;      // the number of the LED pin

const int buttonPin_RGO = 32;     // the number of the pushbutton pin
const int ledPin_RGO =  33;      // the number of the LED pin

const int buttonPin_RSS = 34;     // the number of the pushbutton pin
const int ledPin_RSS =  35;      // the number of the LED pin

const int buttonPin_RDS = 36;     // the number of the pushbutton pin
const int ledPin_RDS =  37;      // the number of the LED pin

///////////////////////////////////////////////
////    RELAYS / RELAY PIN NUMBERS
///////////////////////////////////////////////
const int relayPin_LGH = 38;
const int relayPin_RGH = 39;
const int relayPin_PUMP = 40;
const int relayPin_TANK = 41;

///////////////////////////////////////////////
////    TANK SENSOR PIN
///////////////////////////////////////////////
const int tankSensorPin = 42;

void message(String msg);
void updateAndSaveSettings();
unsigned long readFlowmeterLGH();
void resetFlowmeterLGH();
unsigned long readFlowmeterRGH();
void resetFlowmeterRGH();

//PUMP OBJECT
Pump pump(relayPin_PUMP,
          message,
          &state);

//TANK OBJECT
Tank tank(tankSensorPin,
          relayPin_TANK,
          settings.Data.tankUpdateTime,
          settings.Data.tankDelayTime,
          message,
          &state);//pin, check tank level every

Group groupLEFT(relayPin_LGH,
                settings.Data.pulsesSS_LGH,
                settings.Data.pulsesDS_LGH,
                buttonPin_LST,
                ledPin_LST,
                buttonPin_LGO,
                ledPin_LGO,
                buttonPin_LSS,
                ledPin_LSS,
                buttonPin_LDS,
                ledPin_LDS,
                message,
                resetFlowmeterLGH,
                readFlowmeterLGH,
                updateAndSaveSettings,//UPTO: Making a way to save settings from wihin the class
                &state
               );
Group groupRIGHT(relayPin_RGH,
                 settings.Data.pulsesSS_RGH,
                 settings.Data.pulsesDS_RGH,
                 buttonPin_RST,
                 ledPin_RST,
                 buttonPin_RGO,
                 ledPin_RGO,
                 buttonPin_RSS,
                 ledPin_RSS,
                 buttonPin_RDS,
                 ledPin_RDS,
                 message,
                 resetFlowmeterRGH,
                 readFlowmeterRGH,
                 updateAndSaveSettings,
                 &state
                );


//Set Default State
void defaultState() {

}

///////////////////////////////////////////////////////////////////
///       COMMAND HANDLERS
///////////////////////////////////////////////////////////////////

void Cmd_Unknown() {
  // Allocate the JSON document
  StaticJsonDocument<200> JsonResponse;
  JsonResponse["success"] = false;
  JsonResponse["message"] = "I don't understand";
  serializeJsonPretty(JsonResponse, Serial);
}

void Cmd_Reset() {
  // Allocate the JSON document
  StaticJsonDocument<200> JsonResponse;
  JsonResponse["type"] = "config";
  JsonResponse["command"] = "restart";
  JsonResponse["success"] = true;
  JsonResponse["message"] = "Restarting Now";
  serializeJsonPretty(JsonResponse, Serial);
  resetFunc();  //call reset
}


void Cmd_Help(CommandParameter &Parameters) {
  // Allocate the JSON document
  StaticJsonDocument<200> JsonResponse;
  JsonResponse["type"] = "config";
  JsonResponse["command"] = "help";
  JsonResponse["success"] = true;

  JsonObject commands  = JsonResponse.createNestedObject("commands");
  commands["!set pulsesSSLGH 250"] = "Sets the pulses for a Single Shot on the Left Group to 250";
  commands["!set pulsesDSLGH 350"] = "Sets the pulses for a Double Shot on the Left Group to 350";

  commands["!set pulsesSSRGH 250"] = "Sets the pulses for a Single Shot on the Right Group to 250";
  commands["!set pulsesDSRGH 350"] = "Sets the pulses for a Double Shot on the Right Group to 350";

  commands["!set tankupdatetime 180"] = "Sets the seconds between updating the Tank Level";
  commands["!set tankdelaytime 120"] = "Sets seconds between the tank is LOW and we start filling the tank";

  commands["!get settings"] = "Gets all settings";
  commands["!get state"] = "Gets current state";

  commands["!restart"] = "Restarts the Controller";

  serializeJsonPretty(JsonResponse, Serial);
  Serial.println();
}

void Cmd_Set(CommandParameter &Parameters) {
  // Allocate the JSON document
  StaticJsonDocument<200> JsonResponse;

  //Serial.println(Parameters.NextParameter());
  const char* service = Parameters.NextParameter();
  float setstate = atof(Parameters.NextParameter());

  message("CMD SET SERVICE CHECK->");
  message(service);

  JsonResponse["success"] = true;

  if (strcmp(service, "pulsesSSLGH") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "pulsesSSLGH";
    JsonResponse["message"] = "Pulses Single Shot Left Group Head";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.pulsesSS_LGH = setstate;
      JsonResponse["pulsesSS_LGH"] = settings.Data.pulsesSS_LGH;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set pulsesSS_LGH value was <= 0";
    }
  }
  else if (strcmp(service, "pulsesDSLGH") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "pulsesDSLGH";
    JsonResponse["message"] = "Pulses Double Shot Left Group Head";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.pulsesDS_LGH = setstate;
      JsonResponse["pulsesDS_LGH"] = settings.Data.pulsesDS_LGH;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set pulsesDS_LGH value was <= 0";
    }
  } else if (strcmp(service, "pulsesSSRGH") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "pulsesSSRGH";
    JsonResponse["message"] = "Pulses Single Shot Right Group Head";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.pulsesSS_RGH = setstate;
      JsonResponse["pulsesSS_RGH"] = settings.Data.pulsesSS_RGH;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set pulsesSS_RGH value was <= 0";
    }
  } else if (strcmp(service, "pulsesDSRGH") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "pulsesDSRGH";
    JsonResponse["message"] = "Pulses Double Shot Right Group Head";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.pulsesDS_RGH = setstate;
      JsonResponse["pulsesDS_RGH"] = settings.Data.pulsesDS_RGH;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set pulsesDS_RGH value was <= 0";
    }
  } else if (strcmp(service, "tankupdatetime") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "tankupdatetime";
    JsonResponse["message"] = "Seconds to update the tank i.e check level";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.tankUpdateTime = setstate * 1000;
      JsonResponse["tankUpdateTime"] = settings.Data.tankUpdateTime / 1000;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set tankUpdateTime value was <= 0";
    }
  } else if (strcmp(service, "tankdelaytime") == 0) {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "tankdelaytime";
    JsonResponse["message"] = "Seconds we can wait after we know it is LOW before filling the tank";

    message(StringSumHelper(setstate));

    if (setstate > 0) {
      settings.Data.tankDelayTime = setstate * 1000;
      JsonResponse["tankDelayTime"] = settings.Data.tankDelayTime / 1000;
      settings.Save();
    } else {
      JsonResponse["success"] = false;
      JsonResponse["message"] = "Could Not Set tankDelayTime value was <= 0";
    }
  } else {
    JsonResponse["type"] = "set";
    JsonResponse["command"] = "unknown";
    JsonResponse["success"] = false;
    JsonResponse["message"] = strcat ("No valid service specified: ", service);
  }

  serializeJsonPretty(JsonResponse, Serial);
  Serial.println();
}

void Cmd_Get(CommandParameter &Parameters) {
  // Allocate the JSON document
  StaticJsonDocument<200> JsonResponse;

  //Serial.println(Parameters.NextParameter());
  const char* service = Parameters.NextParameter();
  message("CHECK->");
  message(service);

  JsonResponse["success"] = true;
  if (strcmp(service, "settings") == 0) {
    JsonResponse["type"] = "get";
    JsonResponse["command"] = "settings";
    JsonResponse["message"] = "All Settings";
    JsonResponse["pulsesSS_LGH"] = settings.Data.pulsesSS_LGH;
    JsonResponse["pulsesDS_LGH"] = settings.Data.pulsesDS_LGH;
    JsonResponse["pulsesSS_RGH"] = settings.Data.pulsesSS_RGH;
    JsonResponse["pulsesDS_RGH"] = settings.Data.pulsesDS_RGH;
    JsonResponse["tankUpdateTime"] = settings.Data.tankUpdateTime / 1000;
    JsonResponse["tankDelayTime"] = settings.Data.tankDelayTime / 1000;
  } else if (strcmp(service, "state") == 0) {
    JsonResponse["type"] = "get";
    JsonResponse["command"] = "state";
    JsonResponse["message"] = "Current state";

    JsonResponse["pulsesSS_LGH"] = state.pulsesSS_LGH;
    JsonResponse["pulsesDS_LGH"] = state.pulsesDS_LGH;
    JsonResponse["pulsesSS_RGH"] = state.pulsesSS_RGH;
    JsonResponse["pulsesDS_RGH"] = state.pulsesDS_RGH;
    JsonResponse["tankUpdateTime"] = state.tankUpdateTime;
    JsonResponse["tankDelayTime"] = state.tankDelayTime;

    JsonResponse["pump on for tank:"] = pump.pumpOnFor("tank");
    JsonResponse["pump on for groupRIGHT:"] = pump.pumpOnFor("groupRIGHT");
    JsonResponse["pump on for groupLEFT:"] = pump.pumpOnFor("groupLEFT");

    JsonResponse["groupRIGHT extracting:"] = groupRIGHT.extracting;
    JsonResponse["groupRIGHT extractingSS:"] = groupRIGHT.extractingSS;
    JsonResponse["groupRIGHT extractingDS:"] = groupRIGHT.extractingDS;

    JsonResponse["groupLEFT extracting:"] = groupLEFT.extracting;
    JsonResponse["groupLEFT extractingSS:"] = groupLEFT.extractingSS;
    JsonResponse["groupLEFT extractingDS:"] = groupLEFT.extractingDS;

    JsonResponse["tank full:"] = tank.full;


  } else {
    JsonResponse["type"] = "get";
    JsonResponse["command"] = "unknown";
    JsonResponse["success"] = false;
    JsonResponse["message"] = strcat ("No valid service specified: ", service);
  }

  serializeJsonPretty(JsonResponse, Serial);
  Serial.println();
}

///////////////////////////////////////////////////////////////////
///       SETUP       SETUP      SETUP      SETUP
///////////////////////////////////////////////////////////////////
void setup() {
  //Setup Startup State
  defaultState();

  //Need to uncomment to update EEPROM if we change the stored values, the comment out again
  //settings.Reset();
  //settings.Save();

  state.pulsesSS_LGH = settings.Data.pulsesSS_LGH;
  state.pulsesDS_LGH = settings.Data.pulsesDS_LGH;

  state.pulsesSS_RGH = settings.Data.pulsesSS_RGH;
  state.pulsesDS_RGH = settings.Data.pulsesDS_RGH;

  state.tankUpdateTime = settings.Data.tankUpdateTime; //2 minutes
  state.tankDelayTime = settings.Data.tankDelayTime;//3 minutes

  //Flow Sensors
  pinMode(flowSensorPin_LGH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin_LGH), recordFlowPulse_LGH, RISING);//Configures interrupt 1 (pin 21 on the Arduino Mega) to run the function "recrodFlowPulse"
  pinMode(flowSensorPin_RGH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin_RGH), recordFlowPulse_RGH, RISING);//Configures interrupt 1 (pin 21 on the Arduino Mega) to run the function "recrodFlowPulse"

  interrupts();   //Enables interrupts on the Arduino

  ////////////////////////////////////////////////////////////////

  // Start the Serial
  Serial.begin(115200);
  message("Espresso Controller 0.01a");

  ///SETUP COMMAND HANDLER
  SerialCommandHandler.AddCommand(F("help"), Cmd_Help);
  SerialCommandHandler.AddCommand(F("get"), Cmd_Get);
  SerialCommandHandler.AddCommand(F("set"), Cmd_Set);
  //3

  SerialCommandHandler.AddCommand(F("restart"), Cmd_Reset);
  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
  //5
  ///END SETUP COMMAND HANDLER

  lastMessage = millis();
}

///////////////////////////////////////////////////////////
///////   LOOP      LOOP      LOOP      LOOP      LOOP
//////////////////////////////////////////////////////////
void loop ()
{
  // Check for serial commands and dispatch them.
  SerialCommandHandler.Process();

  pump.update();
  tank.update();
  groupLEFT.update();
  groupRIGHT.update();

  //Manage Tank
  if (!tank.full && !pump.pumpOnFor("tank")) {
    if (!tank.filling) {
      message("Turning Pump On To Fill Tank");
      pump.pumpOn("tank");
      tank.pumpOn();//tell tank we are filling it
    }
  } else if (tank.full && pump.pumpOnFor("tank")) {
    message("Turning Pump Off");
    pump.pumpOff("tank");
    tank.pumpOff();//tell tank we are not filling it
  }

  //Manage Pump for Left Group
  if (groupLEFT.extracting && !pump.pumpOnFor("groupLEFT")) {
    //if group is extracting start pump
    message("Turning Pump On for Left Group Head");
    pump.pumpOn("groupLEFT");
  } else if (!groupLEFT.extracting && pump.pumpOnFor("groupLEFT")) {
    //if group is NOT extracting stop pump
    message("Turning Pump Off for Left Group Head");
    pump.pumpOff("groupLEFT");
  }

  //Manage Pump for Right Group
  if (groupRIGHT.extracting && !pump.pumpOnFor("groupRIGHT")) {
    //if group is extracting start pump
    message("Turning Pump On for Right Group Head");
    pump.pumpOn("groupRIGHT");
  } else if (!groupRIGHT.extracting && pump.pumpOnFor("groupRIGHT")) {
    //if group is NOT extracting stop pump
    message("Turning Pump Off for Right Group Head");
    pump.pumpOff("groupRIGHT");
  }


  //  if (millis() - lastMessage > periodicMessage ){
  //    messageCountTime += periodicMessage;
  //    message(StringSumHelper(messageCountTime/1000) + " " + StringSumHelper(freeMemory()));
  //    lastMessage = millis();
  //  }
}

///////////////////////////////////////////////////////////
////  FLOWMETER FUNCTIONS
///////////////////////////////////////////////////////////

void resetFlowmeterLGH() {
  noInterrupts();
  flowSensorPulses_LGH = 0;      // Reset the counter so we start counting from 0 again
  interrupts();
  message("RESET SENSOR PULSES FOR LGH");
}

unsigned long readFlowmeterLGH() {
  unsigned long flowSensorPulsesNonvolatile;
  noInterrupts();
  flowSensorPulsesNonvolatile = flowSensorPulses_LGH;
  interrupts();
  return flowSensorPulsesNonvolatile;
}

void resetFlowmeterRGH() {
  noInterrupts();
  flowSensorPulses_RGH = 0;      // Reset the counter so we start counting from 0 again
  interrupts();
  message("RESET SENSOR PULSES FOR LGH");
}

unsigned long readFlowmeterRGH() {
  unsigned long flowSensorPulsesNonvolatile;
  noInterrupts();
  flowSensorPulsesNonvolatile = flowSensorPulses_RGH;
  interrupts();
  return flowSensorPulsesNonvolatile;
}
///////////////////////////////////////////////////////////
////  INTERRUPT SERVICE ROUTINGES ISRs for Flowmeters
///////////////////////////////////////////////////////////
void recordFlowPulse_LGH () {
  flowSensorPulses_LGH++;
}
void recordFlowPulse_RGH () {
  flowSensorPulses_RGH++;
}

///////////////////////////////////////////////////////////
////  SERIAL LOGGING
///////////////////////////////////////////////////////////

void error( String err) {
#ifdef DEBUG
  Serial.print("### ERROR: ");
  Serial.println(err);
#endif
}
void message( String msg) {
#ifdef DEBUG
  Serial.println(msg);
#endif
}
void updateAndSaveSettings() { //updates the state and settings from objects and saves
  //Update from Objects
  state.pulsesSS_LGH = groupLEFT.pulsesSS;
  state.pulsesDS_LGH = groupLEFT.pulsesDS;
  state.pulsesSS_RGH = groupRIGHT.pulsesSS;
  state.pulsesDS_RGH = groupRIGHT.pulsesDS;
  state.tankUpdateTime = tank.updateTime;
  state.tankDelayTime = tank.delayTime;
  //Update State
  settings.Data.pulsesSS_LGH = state.pulsesSS_LGH;
  settings.Data.pulsesDS_LGH = state.pulsesDS_LGH;
  settings.Data.pulsesSS_RGH = state.pulsesSS_RGH;
  settings.Data.pulsesDS_RGH = state.pulsesDS_RGH;
  settings.Data.tankUpdateTime = state.tankUpdateTime;
  settings.Data.tankDelayTime = state.tankDelayTime;
  //Save State
  settings.Save();
}
