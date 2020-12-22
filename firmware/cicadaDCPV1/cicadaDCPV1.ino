/**
  System Config (must come before #include "DCPSystem.h")
*/  
#include "src/system/DCPSystem.h"

DCPSystem cicadaDcpSystem;

void setup() {
  //PreInitialization
  cicadaDcpSystem.preInitSystem();
  
  //Start comunication
  cicadaDcpSystem.initCommunication();
   
  // Init all system configurations
  cicadaDcpSystem.initSystem();
}

void loop() {
  if (digitalRead(PIN_AP_WIZARD) == HIGH ) {      
      cicadaDcpSystem.setupWizard();      
   }
}
