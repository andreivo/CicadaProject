/**
  System Config (must come before #include "DCPSystem.h")
*/  
#include "src/system/DCPSystem.h"

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
                    "coreTask", /* nome da tarefa */
                    10000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,          /* prioridade da tarefa (0 a N) */
                    &coreTask,       /* referência para a tarefa (pode ser NULL) */
                    0);         /* Núcleo que executará a tarefa */
}

void loop() {
  cicadaDcpSystem.checkAPWizard(coreTask);
  cicadaDcpSystem.blinkStatus();
  cicadaDcpSystem.readSensors();
}

void loop2(void * pvParameters ){
  cicadaDcpSystem.loopCore2(); 
}
