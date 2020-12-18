#include "src/system/DCPSystem.h"

DCPSystem dcpSystem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  dcpSystem.test();
}
