/**
  System Config (must come before #include "DCPSystem.h")
*/  
#include "src/system/DCPSystem.h"
#include <esp_task_wdt.h> //Biblioteca do watchdog

DCPSystem cicadaDcpSystem;
xTaskHandle coreTask;

void setup() {
  //PreInitialization
  cicadaDcpSystem.preInitSystem();
  
  //Start comunication
  cicadaDcpSystem.initCommunication();
   
  // Init all system configurations
  cicadaDcpSystem.initSystem();
  
  xTaskCreatePinnedToCore(
                    loop2,   /* função que implementa a tarefa */
                    "transmitFunctionsLoop", /* nome da tarefa */
                    20000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,          /* prioridade da tarefa (0 a N) */
                    &coreTask,       /* referência para a tarefa (pode ser NULL) */
                    0);         /* Núcleo que executará a tarefa */

/* Internet connection may take seconds to complete. This delay can block the main loop.
   In the case of the PubSubClient library, the default connection timeout is 15 seconds.
   To prevent the Task Watchdog from being triggered (default 5 seconds) it is necessary to change the trigger time to 60 seconds.*/
  esp_task_wdt_init(60, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  cicadaDcpSystem.checkAPWizard(coreTask);
  cicadaDcpSystem.blinkStatus();
  cicadaDcpSystem.readSensors();

  //Update Task Watchdog timer
  esp_task_wdt_reset();
}

void loop2(void * pvParameters ){
  cicadaDcpSystem.transmitFunctionsLoop(); 
}
