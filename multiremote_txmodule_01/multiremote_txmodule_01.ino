/**************************************************************************************
 * 
 * Multiremote
 * ===========
 * a remote for rc function models with multiple digital and analog inputs via serial transmission (433 Mhz or 2,4 GHz).
 * Custom tx modules provide special controls for different use cases (crane, fire monitor, nautical lights, etc.)
 * Digital Servo functionality by Pikoder SSC (www.pikoder.de)
 * 
 * 
 * Slave module #01 for Multiremote (crane 1)
 * ==========================================
 * 
 * Slave module gathers input from digital or analog sources and sends it to a master device via i2c.
 * Module layout can be individual configured (configuration must be shared with master unit)
 * 
 * INPUTS:
 * 2x analog in
 * 6x digital in / 8 x digital in possible
 * 
 * Written by Kai Laborenz with the help of all the beautifull examples out there
 * This is Open Source: GNU GPL v3 https://choosealicense.com/licenses/gpl-3.0/
 * 
 **************************************************************************************/

#include <Wire.h>
#include <i2cSimpleTransfer.h>

#define i2c_slave_address 1
#define TURN_PIN A0 // input for crane turn control
#define LIFT_PIN A1 // input for crane lifting 

const int thr = 3;  // threshold to compensate for servo input fluctuations

// module layout - must be configured in the master sketch as well
struct SLAVE_DATA {
    uint8_t lift = 0;   // crane turn
    uint8_t turn = 0;   // crane lifting
    byte switches = 0;  // 8 switches can be transmitted with one byte
};

uint8_t sensorValue,sensorValue2,oldValue,oldValue2 = 0;

byte localSwitch[8]; // local state of switches

SLAVE_DATA slave_data;

void setup() {
  Serial.begin(9600);       
  Wire.begin(i2c_slave_address);    // join i2c as a slave
  Wire.onRequest(sendData);         // when the Master makes a request, run this function
  Serial.println(" Start ");
  
  u8g2.begin(/*Select=*/ 0, /*Right/Next=*/ 1, /*Left/Prev=*/ 2, /*Up=*/ 4, /*Down=*/ 3, /*Home/Cancel=*/ A6); 

  // set D2 - D9 as input for switches
  for(int i=0; i<7; i++){
    pinMode(i+2, INPUT);
  }
}

void loop() {

}

void sendData() {
  
    // read lift control
    sensorValue = map(analogRead(LIFT_PIN),0,1023,0,255);
    if ((sensorValue > (oldValue+thr)) ||  (sensorValue < (oldValue-thr))) {
      slave_data.lift = sensorValue;
      oldValue = sensorValue;
    }
      else {
           slave_data.lift = oldValue;
     }
    // read turn control
    sensorValue2 = map(analogRead(TURN_PIN),0,1023,0,255);
    if ((sensorValue2 > (oldValue2+thr)) ||  (sensorValue2 < (oldValue2-thr))) {
      slave_data.turn = sensorValue2;
      oldValue2 = sensorValue2;
    }
      else {
           slave_data.turn = oldValue2;
     }
     // read 8 switches D2 - D9 and store them locally
      slave_data.switches = 0;
      for(int i=0; i<=7; i++){
        localSwitch[i] = digitalRead(i+2);
        Serial.print(localSwitch[i]);
        Serial.print(", ");
        slave_data.switches |= localSwitch[i] << i;   // 8 switch status vueles (on/off) can be stored in one byte
      }     
      Serial.print(" || ");
      // testing the byte value
      for (int i=0; i<=7; i++) {
        bool b = bitRead(slave_data.switches, i);
        Serial.print(b);
      }
      Serial.print(" || ");
     
      Serial.print(slave_data.lift);
      Serial.print(" | ");
      Serial.print(slave_data.turn);
      Serial.print(" | ");
      Serial.println(slave_data.switches,BIN);
      
      i2cSimpleWrite(slave_data);            // Send the Master the data
}
