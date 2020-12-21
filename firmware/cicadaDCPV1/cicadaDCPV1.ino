/**
  System Config (must come before #include "DCPSystem.h")
*/  
#define CIC_DEBUG_ENABLED true
#define CIC_SYSTEM_BAUDRATE 115200
#include "src/system/DCPSystem.h"

DCPSystem dcpSystem;

void setup() {
  // Init the Serial    
  CIC_DEBUG_SETUP(CIC_SYSTEM_BAUDRATE);  
  CIC_DEBUG_(F("\n\nCICADA DCP FIRMWARE (Version "));
  CIC_DEBUG_(dcpSystem.getFwmVersion());
  CIC_DEBUG(F(")"));

  // Initialize Station ID  
  dcpSystem.initStationID();

  // Initialize Station Name
  dcpSystem.initStationName();
  
  // Register Firmware Version
  dcpSystem.initFirmwareVersion();
  //Show all config
  dcpSystem.printConfiguration();

  //Start Wizard configuration
  dcpSystem.setupWizard();
   
  // Init all system configurations
  dcpSystem.initSystem();
}

void loop() {
  // put your main code here, to run repeatedly:
//  dcpSystem.test();
}
