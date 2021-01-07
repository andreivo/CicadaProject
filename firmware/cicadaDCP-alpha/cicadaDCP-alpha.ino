/**
 * CICADA DCP Firmware for the ESP32
 *
 *    VERSION: 0.0.1-alpha
 *    DATE   : 2021-01
 *    AUTHORS: André Ivo <andre.ivo@gmail.com.br>
 *    LICENSE: CC-BY-4.0
 *       SITE: https://github.com/andreivo/CicadaProject
 */
 
#include "src/System/DCPSystem.h"
#include <esp_task_wdt.h> //Watchdog lib

DCPSystem cicadaDcpSystem;
xTaskHandle coreTask;

void setup() {
  //Disable Log on initialization
  logDisable();
  
  // PreInitialization
  cicadaDcpSystem.preInitSystem();
  
  // Start comunication
  cicadaDcpSystem.initCommunication();
   
  // Init all system configurations
  cicadaDcpSystem.initSystem();

  // Create a new parallel task on core 0
  // The task that is running in parallel transmit data to server MQTT               
  xTaskCreatePinnedToCore (
                     taskTransmitLoop,   /* function that implements the task */
                     "taskTransmitLoop", /* task name */
                     20000,              /* number of words to be allocated for use with the task stack */
                     NULL,               /* input parameter for the task (can be NULL) */
                     1,                  /* task priority (0 to N) */
                     &coreTask,          /* reference to the task (can be NULL) */
                     0);

  /* Internet connection may take seconds to complete. This delay can block the main loop.
     In the case of the PubSubClient library, the default connection timeout is 15 seconds.
     To prevent the Task Watchdog from being triggered (default 5 seconds) it is necessary to 
     change the trigger time to 120 seconds.*/
  esp_task_wdt_init(120, true);
  esp_task_wdt_add(NULL);

  //Enable Log after initialization
  logEnable();
}

void loop() {
  cicadaDcpSystem.readSerialCommands(coreTask);
  cicadaDcpSystem.checkAPWizard(coreTask);
  cicadaDcpSystem.blinkStatus();
  cicadaDcpSystem.readSensors();

  //Update Task Watchdog timer
  esp_task_wdt_reset();
}

void taskTransmitLoop(void * pvParameters ){
  cicadaDcpSystem.taskTransmitLoop(); 
}
