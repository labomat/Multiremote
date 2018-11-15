# Multiremote
Remote Control for rc function models with individual configurable user interface

Build for special functions of rc controlled models with individual configurable function modules on the transmitter.

Each module consists of an Arduino, some input controls (potentiometer, rotary switch, switches, buttons) and an oled display (optional).
Control input is send via i2c to a master device. The master device combines data from all modules and generated a mini-ssc serial signal that is delivered to the receiver.
On the receiver side there are ecoders that drive servos and switches. 

At the moment the repository consists only of transmitter modules. I am using the Pikoder (www.pikoder.de) as encoder device and servo driver and a 433MHz module to transmit serial data.

*-= WORK IN PROGRESS =-*
