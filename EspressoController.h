#ifndef espressoControllerTypes
#define espressoControllerTypes
///////////////////////////////////////////////
////    STATE STRUCT
///////////////////////////////////////////////
struct espressoState {

  unsigned long pulsesSS_LGH = 250;
  unsigned long pulsesDS_LGH = 350;
  
  unsigned long pulsesSS_RGH = 250;
  unsigned long pulsesDS_RGH = 350;

  unsigned long tankUpdateTime = 120000; //2 minutes
  unsigned long tankDelayTime = 180000;//3 minutes 

  unsigned long flowCheckTime = 100;//.1 seconds
  unsigned long extractionTimeout = 90000;//1.5 minutes
};
#endif
