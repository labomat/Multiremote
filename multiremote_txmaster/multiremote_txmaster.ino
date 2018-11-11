/**************************************************************************************
 * 
 * Multiremote
 * ===========
 * a remote for rc function models with multiple digital and analog inputs via serial transmission (433 Mhz or 2,4 GHz).
 * Custom tx modules provide special controls for different use cases (crane, fire monitor, nautical lights, etc.)
 * Digital Servo functionality by Pikoder SSC (www.pikoder.de)
 * 
 * 
 * Master module für Multiremote
 * ================================
 * 
 * The Master module collects input from multiple Slave modules via i2c and delivers 
 * serial data for a Mini-SSC controller (Pikoder - www.pikoder.de)
 * 
 * Modules have individual configuration that must be known to the master.
 * 
 * Written by Kai Laborenz with the help of all the beautifull examples out there
 * This is Open Source: GNU GPL v3 https://choosealicense.com/licenses/gpl-3.0/
 *  
 **************************************************************************************/

#include <SoftwareSerial.h>
#include <Wire.h>
#include <i2cSimpleTransfer.h>

// only need for local servo (testing)
#include <Servo.h>
Servo myservo, myservo2;  

// define individual adress for all slave modules
#define i2c_sensor_slave 1
//#define i2c_sensor_slave2 17

// layout for module #01 (crane 1)
struct SLAVE_DATA {
    uint8_t lift;   // crane turn
    uint8_t turn;   // crane lifting
    byte switches;  // 8 switches can be transmitted with one byte
};

// layout for module #02 
// You can add more variables into the struct, but the default limit for transfer size in the Wire library is 32 bytes
struct SLAVE2_DATA {
    uint8_t sensor;  // use specific declarations to ensure communication between 16bit and 32bit controllers
};

#define tx_PIN 5  // tx pin to SSC
#define rx_PIN 6  // rx pin from SSC (not used)

// Software Seriel for communication with SSC controller
SoftwareSerial ssc = SoftwareSerial(rx_PIN, tx_PIN);

SLAVE_DATA slave_data;
SLAVE2_DATA slave2_data;

byte slave1Switches[8];

void setup() {
    Wire.begin();         // start i2c as Master
    Serial.begin(9600);   // start local serial (for debugging)
    ssc.begin(9600);      // start SoftwareSerial (for communication with SSC)

    // only need for local testing
    myservo.attach(9); // servos
    myservo2.attach(8);
    pinMode(2, OUTPUT); // leds
    pinMode(3, OUTPUT);
    
}

void loop() {

  // request data from the moddule #01 the size of defined struct
  Wire.requestFrom( i2c_sensor_slave, sizeof(slave_data) );    

    if ( Wire.available() == sizeof(slave_data) ) {
        i2cSimpleRead(slave_data);
    }
    

//    ToDo  Test a second slave device
//    Wire.requestFrom(i2c_sensor_slave2, sizeof(slave2_data));    // request data from the Slave2 device the size of our struct
//    if ( Wire.available() == sizeof(slave2_data) ) {
//        i2cSimpleRead(slave2_data);
//    }


//    Output 1 for testing: direct servo operation
//
//    int val = map(slave_data.lift, 0, 255, 0, 180);
//    myservo.write(val);
//
//    int val2 = map(slave_data.turn, 0, 255, 0, 180);
//    myservo2.write(val2);


    // Output 2: send data to Mini-SSC module via SoftwarSerial
    //
    ssc.write(byte(0xFF));              // SSC starting code
    ssc.write(byte(4));                 // number of Servo (0-7) - or 0-255 if more than one ssc controller is used (daisy chained)
    ssc.write(byte(slave_data.lift));   // control data for servo (0-255)
    
    ssc.write(byte(0xFF));
    ssc.write(byte(3));
    ssc.write(byte(slave_data.turn));

    // 8 binary values (switch status) from one byte
    // read bit by bit and saved into array 
    for (int i=0; i<=7; i++) {
      slave1Switches[i] = bitRead(slave_data.switches, i);
      Serial.print(slave1Switches[i]);
    }

//    Serial.print(" || ");
//    Serial.print(val);
//    Serial.print(" | ");
//    Serial.println(val2);
//    Serial.print(" | ");
//    Serial.print(slave_data.ropeUp);
//    Serial.print(" | ");
//    Serial.println(slave_data.ropeDown);

//  show transmitted switch status via leds
    digitalWrite(2,slave1Switches[0]);
    digitalWrite(3,slave1Switches[5]);
 
    delay(10);     // Just for example use.  Use some sort of timed action for implementation
}
